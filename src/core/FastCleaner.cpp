#include "FastCleaner.h"
#include "I18n.h"
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QElapsedTimer>
#include <QtConcurrent/QtConcurrent>
#include <QDebug>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

QString CleanResult::formattedBytes() const
{
    if (bytesFreed < 1024) {
        return QString("%1 %2").arg(bytesFreed).arg(trText("unit_bytes"));
    } else if (bytesFreed < 1024 * 1024) {
        return QString("%1 %2").arg(QString::number(bytesFreed / 1024.0, 'f', 1)).arg(trText("unit_kb"));
    } else if (bytesFreed < 1024 * 1024 * 1024) {
        return QString("%1 %2").arg(QString::number(bytesFreed / (1024.0 * 1024.0), 'f', 2)).arg(trText("unit_mb"));
    } else {
        return QString("%1 %2").arg(QString::number(bytesFreed / (1024.0 * 1024.0 * 1024.0), 'f', 2)).arg(trText("unit_gb"));
    }
}

QString CleanResult::summary() const
{
    QString typeStr;
    switch (type) {
    case CleanType::BinAndDbo: typeStr = trText("clean_type_bin_dbo"); break;
    case CleanType::Lightmaps: typeStr = trText("clean_type_lightmaps"); break;
    case CleanType::All: typeStr = trText("clean_type_all"); break;
    case CleanType::LevelBuildData: typeStr = trText("clean_type_level_data"); break;
    case CleanType::EngineTemp: typeStr = trText("clean_type_engine_temp"); break;
    }

    if (filesDeleted == 0) {
        return QString(trText("clean_summary_empty")).arg(typeStr).arg(elapsedMs);
    }

    return QString(trText("clean_summary_clean"))
        .arg(filesDeleted)
        .arg(formattedBytes())
        .arg(elapsedMs);
}

FastCleaner& FastCleaner::instance()
{
    static FastCleaner s_instance;
    return s_instance;
}

FastCleaner::FastCleaner(QObject *parent)
    : QObject(parent)
{
    m_watcher = new QFutureWatcher<CleanResult>(this);
    connect(m_watcher, &QFutureWatcher<CleanResult>::finished, this, [this]() {
        CleanResult res = m_watcher->result();
        emit cleanFinished(res.type, res);
    });
}

void FastCleaner::startCleanAsync(const QString &fpscDir, CleanType type)
{
    emit cleanStarted(type);
    QFuture<CleanResult> future = QtConcurrent::run(&FastCleaner::cleanSync, fpscDir, type);
    m_watcher->setFuture(future);
}

CleanResult FastCleaner::cleanSync(const QString &fpscDir, CleanType type)
{
    switch (type) {
    case CleanType::BinAndDbo:
        return cleanBinAndDbo(fpscDir);
    case CleanType::Lightmaps:
        return cleanLightmaps(fpscDir);
    case CleanType::All:
        return cleanAll(fpscDir);
    case CleanType::LevelBuildData:
        return cleanLevelBuildData(fpscDir);
    case CleanType::EngineTemp:
        return cleanEngineTemp();
    }
    return CleanResult();
}

CleanResult FastCleaner::cleanBinAndDbo(const QString &fpscDir)
{
    CleanResult result;
    result.type = CleanType::BinAndDbo;

    if (fpscDir.isEmpty()) {
        result.success = false;
        result.errorMessage = trText("err_folder_not_set");
        return result;
    }

    QDir rootDir(fpscDir);
    QString filesPath = rootDir.filePath("Files");
    if (!QDir(filesPath).exists()) {
        filesPath = fpscDir; // fallback
    }

    QElapsedTimer timer;
    timer.start();

    // Fast recursive iterator for .bin and .dbo
    QStringList nameFilters;
    nameFilters << "*.bin" << "*.dbo" << "*.BIN" << "*.DBO";

    QDirIterator it(filesPath, nameFilters, QDir::Files | QDir::NoSymLinks | QDir::Hidden, QDirIterator::Subdirectories);

    while (it.hasNext()) {
        it.next();
        QFileInfo info = it.fileInfo();
        qint64 size = info.size();
        QString path = info.absoluteFilePath();

#ifdef Q_OS_WIN
        // Fast Win32 file deletion
        if (DeleteFileW(reinterpret_cast<LPCWSTR>(path.utf16()))) {
            result.filesDeleted++;
            result.bytesFreed += size;
        } else {
            // Fallback to QFile::remove
            if (QFile::remove(path)) {
                result.filesDeleted++;
                result.bytesFreed += size;
            }
        }
#else
        if (QFile::remove(path)) {
            result.filesDeleted++;
            result.bytesFreed += size;
        }
#endif
    }

    result.elapsedMs = timer.elapsed();
    return result;
}

CleanResult FastCleaner::cleanLightmaps(const QString &fpscDir)
{
    CleanResult result;
    result.type = CleanType::Lightmaps;

    if (fpscDir.isEmpty()) {
        result.success = false;
        result.errorMessage = trText("err_folder_not_set");
        return result;
    }

    QElapsedTimer timer;
    timer.start();

    QDir rootDir(fpscDir);
    QString testLevelPath = rootDir.filePath("Files/levelbank/testlevel");
    if (!QDir(testLevelPath).exists()) {
        testLevelPath = rootDir.filePath("levelbank/testlevel");
    }

    if (QDir(testLevelPath).exists()) {
        // 1. Clean lightmaps subfolder
        QString lmDir = QDir(testLevelPath).filePath("lightmaps");
        if (QDir(lmDir).exists()) {
            QDirIterator it(lmDir, QDir::Files | QDir::NoSymLinks | QDir::Hidden, QDirIterator::Subdirectories);
            while (it.hasNext()) {
                it.next();
                QFileInfo info = it.fileInfo();
                qint64 size = info.size();
                QString path = info.absoluteFilePath();
#ifdef Q_OS_WIN
                if (DeleteFileW(reinterpret_cast<LPCWSTR>(path.utf16())) || QFile::remove(path)) {
                    result.filesDeleted++;
                    result.bytesFreed += size;
                }
#else
                if (QFile::remove(path)) {
                    result.filesDeleted++;
                    result.bytesFreed += size;
                }
#endif
            }
        }

        // 2. Clean lightmap texture artifacts in testlevel (e.g. lightmap_*.dds, lightmap_*.bmp, *.png, *.lm*)
        QStringList lmFilters;
        lmFilters << "lightmap_*.dds" << "lightmap_*.bmp" << "lightmap_*.png" 
                  << "lm_*.dds" << "lm_*.bmp" << "*.lm" << "*.lmb" << "lightmaps.dat";
        
        QDirIterator testIt(testLevelPath, lmFilters, QDir::Files | QDir::NoSymLinks, QDirIterator::NoIteratorFlags);
        while (testIt.hasNext()) {
            testIt.next();
            QFileInfo info = testIt.fileInfo();
            qint64 size = info.size();
            QString path = info.absoluteFilePath();
#ifdef Q_OS_WIN
            if (DeleteFileW(reinterpret_cast<LPCWSTR>(path.utf16())) || QFile::remove(path)) {
                result.filesDeleted++;
                result.bytesFreed += size;
            }
#else
            if (QFile::remove(path)) {
                result.filesDeleted++;
                result.bytesFreed += size;
            }
#endif
        }
    }

    result.elapsedMs = timer.elapsed();
    return result;
}

CleanResult FastCleaner::cleanAll(const QString &fpscDir)
{
    QElapsedTimer timer;
    timer.start();

    CleanResult resBin = cleanBinAndDbo(fpscDir);
    CleanResult resLevel = cleanLevelBuildData(fpscDir);
    CleanResult resLm = cleanLightmaps(fpscDir);

    CleanResult total;
    total.type = CleanType::All;
    total.filesDeleted = resBin.filesDeleted + resLevel.filesDeleted + resLm.filesDeleted;
    total.bytesFreed = resBin.bytesFreed + resLevel.bytesFreed + resLm.bytesFreed;
    total.elapsedMs = timer.elapsed();
    total.success = resBin.success && resLevel.success && resLm.success;
    if (!total.success) {
        if (!resBin.errorMessage.isEmpty()) total.errorMessage = resBin.errorMessage;
        else if (!resLevel.errorMessage.isEmpty()) total.errorMessage = resLevel.errorMessage;
        else total.errorMessage = resLm.errorMessage;
    }
    return total;
}

CleanResult FastCleaner::cleanLevelBuildData(const QString &fpscDir)
{
    CleanResult result;
    result.type = CleanType::LevelBuildData;

    if (fpscDir.isEmpty()) {
        result.success = false;
        result.errorMessage = trText("err_folder_not_set");
        return result;
    }

    QElapsedTimer timer;
    timer.start();

    QDir rootDir(fpscDir);
    QString testLevelPath = rootDir.filePath("Files/levelbank/testlevel");
    if (!QDir(testLevelPath).exists()) {
        testLevelPath = rootDir.filePath("levelbank/testlevel");
    }

    QStringList targetDirs;
    if (QDir(testLevelPath).exists()) {
        targetDirs << testLevelPath;
        QString lmDir = QDir(testLevelPath).filePath("lightmaps");
        if (QDir(lmDir).exists()) {
            targetDirs << lmDir;
        }
    }

    // Delete dbo, ele, lgt, dat from lightmaps and testlevel folder
    QStringList filters;
    filters << "*.dbo" << "*.ele" << "*.lgt" << "*.dat"
            << "*.DBO" << "*.ELE" << "*.LGT" << "*.DAT";

    for (const QString &dirPath : targetDirs) {
        QDirIterator it(dirPath, filters, QDir::Files | QDir::NoSymLinks, QDirIterator::NoIteratorFlags);
        while (it.hasNext()) {
            it.next();
            QFileInfo info = it.fileInfo();
            qint64 size = info.size();
            QString path = info.absoluteFilePath();
#ifdef Q_OS_WIN
            if (DeleteFileW(reinterpret_cast<LPCWSTR>(path.utf16())) || QFile::remove(path)) {
                result.filesDeleted++;
                result.bytesFreed += size;
            }
#else
            if (QFile::remove(path)) {
                result.filesDeleted++;
                result.bytesFreed += size;
            }
#endif
        }
    }

    result.elapsedMs = timer.elapsed();
    return result;
}

CleanResult FastCleaner::cleanEngineTemp()
{
    CleanResult result;
    result.type = CleanType::EngineTemp;

    QElapsedTimer timer;
    timer.start();

    QString tempPath = QDir::tempPath();
    QDir tempDir(tempPath);

    // Look for directories starting with dbpdata
    QStringList nameFilters;
    nameFilters << "dbpdata*" << "DBPDATA*";

    QStringList dirs = tempDir.entryList(nameFilters, QDir::Dirs | QDir::NoDotAndDotDot | QDir::Hidden);

    for (const QString &subDirName : dirs) {
        QString fullSubDirPath = tempDir.filePath(subDirName);

        // Delete all files inside recursively
        QDirIterator fileIt(fullSubDirPath, QDir::Files | QDir::Hidden | QDir::System, QDirIterator::Subdirectories);
        while (fileIt.hasNext()) {
            fileIt.next();
            QFileInfo info = fileIt.fileInfo();
            qint64 size = info.size();
            QString filePath = info.absoluteFilePath();
#ifdef Q_OS_WIN
            if (DeleteFileW(reinterpret_cast<LPCWSTR>(filePath.utf16())) || QFile::remove(filePath)) {
                result.filesDeleted++;
                result.bytesFreed += size;
            }
#else
            if (QFile::remove(filePath)) {
                result.filesDeleted++;
                result.bytesFreed += size;
            }
#endif
        }

        // Try to remove directory itself (ignoring locked directories in use)
        QDir(fullSubDirPath).removeRecursively();
    }

    result.elapsedMs = timer.elapsed();
    return result;
}
