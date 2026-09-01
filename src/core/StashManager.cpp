#include "StashManager.h"
#include "ProcessController.h"
#include "I18n.h"
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QSettings>
#include <algorithm>
#include <QDebug>

QString StashInfo::formattedBytes() const
{
    if (totalBytes < 1024) {
        return QString("%1 %2").arg(totalBytes).arg(trText("unit_bytes"));
    } else if (totalBytes < 1024 * 1024) {
        return QString("%1 %2").arg(QString::number(totalBytes / 1024.0, 'f', 1)).arg(trText("unit_kb"));
    } else {
        return QString("%1 %2").arg(QString::number(totalBytes / (1024.0 * 1024.0), 'f', 2)).arg(trText("unit_mb"));
    }
}

QString StashInfo::formattedTime() const
{
    return createdAt.toString("dd.MM.yyyy HH:mm:ss");
}

StashManager& StashManager::instance()
{
    static StashManager s_instance;
    return s_instance;
}

StashManager::StashManager(QObject *parent)
    : QObject(parent)
{
}

QString StashManager::testLevelDirectory(const QString &fpscDir) const
{
    if (fpscDir.isEmpty()) return "";
    QDir dir(fpscDir);
    QString path = dir.filePath("Files/levelbank/testlevel");
    if (!QDir(path).exists()) {
        path = dir.filePath("levelbank/testlevel");
    }
    return path;
}

QString StashManager::stashesRootDirectory(const QString &fpscDir) const
{
    if (fpscDir.isEmpty()) return "";
    return QDir(fpscDir).filePath("_fpsc_stashes");
}

bool StashManager::removeDirectoryContents(const QString &dirPath)
{
    QDir dir(dirPath);
    if (!dir.exists()) return true;

    QFileInfoList entries = dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot | QDir::Hidden);
    for (const QFileInfo &entry : entries) {
        if (entry.isDir()) {
            QDir subDir(entry.absoluteFilePath());
            if (!subDir.removeRecursively()) {
                return false;
            }
        } else {
            if (!QFile::remove(entry.absoluteFilePath())) {
                return false;
            }
        }
    }
    return true;
}

bool StashManager::copyDirectoryRecursively(const QString &srcPath, const QString &dstPath, int &fileCount, qint64 &totalBytes)
{
    QDir srcDir(srcPath);
    if (!srcDir.exists()) return false;

    QDir dstDir(dstPath);
    if (!dstDir.exists()) {
        if (!dstDir.mkpath(".")) return false;
    }

    QDirIterator it(srcPath, QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot | QDir::Hidden, QDirIterator::Subdirectories);

    while (it.hasNext()) {
        it.next();
        QFileInfo fileInfo = it.fileInfo();
        QString relative = srcDir.relativeFilePath(fileInfo.absoluteFilePath());
        QString targetFilePath = dstDir.filePath(relative);

        if (fileInfo.isDir()) {
            QDir().mkpath(targetFilePath);
        } else {
            QDir().mkpath(QFileInfo(targetFilePath).dir().absolutePath());
            if (QFile::exists(targetFilePath)) {
                QFile::remove(targetFilePath);
            }
            if (QFile::copy(fileInfo.absoluteFilePath(), targetFilePath)) {
                fileCount++;
                totalBytes += fileInfo.size();
            } else {
                return false;
            }
        }
    }

    return true;
}

void StashManager::readStashMeta(const QString &stashDir, StashInfo &info) const
{
    QString metaPath = QDir(stashDir).filePath("stash.meta");
    if (QFile::exists(metaPath)) {
        QSettings settings(metaPath, QSettings::IniFormat);
        info.id = settings.value("id", info.id).toString();
        info.name = settings.value("name", info.name).toString();
        info.createdAt = settings.value("createdAt", info.createdAt).toDateTime();
        info.fileCount = settings.value("fileCount", info.fileCount).toInt();
        info.totalBytes = settings.value("totalBytes", info.totalBytes).toLongLong();
        info.fpmSourcePath = settings.value("fpmSourcePath", info.fpmSourcePath).toString();
        info.fpmRelativePath = settings.value("fpmRelativePath", info.fpmRelativePath).toString();
    }
}

void StashManager::writeStashMeta(const QString &stashDir, const StashInfo &info) const
{
    QString metaPath = QDir(stashDir).filePath("stash.meta");
    QSettings settings(metaPath, QSettings::IniFormat);
    settings.setValue("id", info.id);
    settings.setValue("name", info.name);
    settings.setValue("createdAt", info.createdAt);
    settings.setValue("fileCount", info.fileCount);
    settings.setValue("totalBytes", info.totalBytes);
    settings.setValue("fpmSourcePath", info.fpmSourcePath);
    settings.setValue("fpmRelativePath", info.fpmRelativePath);
    settings.sync();
}

bool StashManager::createStash(const QString &fpscDir, const QString &name, StashInfo *outInfo, QString *outError)
{
    QString testDir = testLevelDirectory(fpscDir);
    if (testDir.isEmpty() || !QDir(testDir).exists()) {
        if (outError) *outError = trText("err_testlevel_not_found");
        return false;
    }

    QString stashesRoot = stashesRootDirectory(fpscDir);
    QDir().mkpath(stashesRoot);

    QDateTime now = QDateTime::currentDateTime();
    QString stashId = now.toString("yyyyMMdd_HHmmss_zzz");
    QString targetStashDir = QDir(stashesRoot).filePath(stashId);

    QString dataDir = QDir(targetStashDir).filePath("data");
    QDir().mkpath(dataDir);

    int fileCount = 0;
    qint64 totalBytes = 0;

    if (!copyDirectoryRecursively(testDir, dataDir, fileCount, totalBytes)) {
        QDir(targetStashDir).removeRecursively();
        if (outError) *outError = trText("err_copy_testlevel");
        return false;
    }

    // Capture currently opened .fpm map file from FPS Creator window title if available
    QString fpmRel;
    QString fpmSrc = ProcessController::instance().findCurrentMapFpmPath(fpscDir, &fpmRel);
    QString savedFpmSrc;
    QString savedFpmRel;

    if (!fpmSrc.isEmpty() && QFile::exists(fpmSrc)) {
        QString mapDir = QDir(targetStashDir).filePath("map");
        QDir().mkpath(mapDir);
        QString dstFpm = QDir(mapDir).filePath(QFileInfo(fpmSrc).fileName());
        if (QFile::copy(fpmSrc, dstFpm)) {
            savedFpmSrc = fpmSrc;
            savedFpmRel = fpmRel;
            fileCount++;
            totalBytes += QFileInfo(fpmSrc).size();
        }
    }

    StashInfo info;
    info.id = stashId;
    info.name = name.isEmpty() ? QString(trText("stash_default_name")).arg(now.toString("dd.MM.yyyy HH:mm:ss")) : name;
    info.createdAt = now;
    info.fileCount = fileCount;
    info.totalBytes = totalBytes;
    info.folderPath = targetStashDir;
    info.fpmSourcePath = savedFpmSrc;
    info.fpmRelativePath = savedFpmRel;

    writeStashMeta(targetStashDir, info);

    if (outInfo) *outInfo = info;
    emit stashSaved(info);
    emit stashesUpdated();
    return true;
}

bool StashManager::quickSaveStash(const QString &fpscDir, QString *outError)
{
    StashInfo info;
    return createStash(fpscDir, "", &info, outError);
}

bool StashManager::restoreStash(const QString &fpscDir, const QString &stashId, QString *outError)
{
    QString testDir = testLevelDirectory(fpscDir);
    if (testDir.isEmpty()) {
        if (outError) *outError = trText("err_folder_not_set");
        return false;
    }

    QString targetId = stashId;
    if (targetId.isEmpty()) {
        QVector<StashInfo> list = listStashes(fpscDir);
        if (list.isEmpty()) {
            if (outError) *outError = trText("err_no_stashes_restore");
            return false;
        }
        targetId = list.first().id;
    }

    QString stashFolder = QDir(stashesRootDirectory(fpscDir)).filePath(targetId);
    QString dataFolder = QDir(stashFolder).filePath("data");

    if (!QDir(dataFolder).exists()) {
        if (outError) *outError = trText("err_stash_not_found");
        return false;
    }

    // 1. Clear current testlevel contents completely
    QDir().mkpath(testDir);
    if (!removeDirectoryContents(testDir)) {
        if (outError) *outError = trText("err_clean_testlevel");
        return false;
    }

    // 2. Copy files from snapshot data folder into testlevel
    int count = 0;
    qint64 bytes = 0;
    if (!copyDirectoryRecursively(dataFolder, testDir, count, bytes)) {
        if (outError) *outError = trText("err_restore_testlevel");
        return false;
    }

    // 3. Restore .fpm map file if present in stash
    QString mapDir = QDir(stashFolder).filePath("map");
    if (QDir(mapDir).exists()) {
        QString metaPath = QDir(stashFolder).filePath("stash.meta");
        QSettings settings(metaPath, QSettings::IniFormat);
        QString fpmSrc = settings.value("fpmSourcePath", "").toString();
        QString fpmRel = settings.value("fpmRelativePath", "").toString();

        QDirIterator mapIt(mapDir, QStringList() << "*.fpm" << "*.FPM", QDir::Files);
        if (mapIt.hasNext()) {
            mapIt.next();
            QString srcFpmFile = mapIt.fileInfo().absoluteFilePath();
            QString targetFpmFile = fpmSrc;
            if (targetFpmFile.isEmpty() && !fpmRel.isEmpty()) {
                targetFpmFile = QDir(fpscDir).filePath("Files/" + fpmRel);
            }
            if (!targetFpmFile.isEmpty()) {
                QDir().mkpath(QFileInfo(targetFpmFile).dir().absolutePath());
                if (QFile::exists(targetFpmFile)) {
                    QFile::remove(targetFpmFile);
                }
                QFile::copy(srcFpmFile, targetFpmFile);
            }
        }
    }

    emit stashRestored(targetId);
    return true;
}

bool StashManager::quickRestoreStash(const QString &fpscDir, QString *outError)
{
    return restoreStash(fpscDir, "", outError);
}

bool StashManager::deleteStash(const QString &fpscDir, const QString &stashId, QString *outError)
{
    if (stashId.isEmpty()) return false;
    QString stashFolder = QDir(stashesRootDirectory(fpscDir)).filePath(stashId);
    QDir dir(stashFolder);
    if (!dir.exists()) {
        if (outError) *outError = trText("err_stash_not_found");
        return false;
    }

    if (!dir.removeRecursively()) {
        if (outError) *outError = trText("err_delete_stash");
        return false;
    }

    emit stashesUpdated();
    return true;
}

QVector<StashInfo> StashManager::listStashes(const QString &fpscDir) const
{
    QVector<StashInfo> result;
    QString root = stashesRootDirectory(fpscDir);
    if (root.isEmpty() || !QDir(root).exists()) {
        return result;
    }

    QDir rootDir(root);
    QFileInfoList entryList = rootDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);

    for (const QFileInfo &entry : entryList) {
        StashInfo info;
        info.id = entry.fileName();
        info.folderPath = entry.absoluteFilePath();
        info.createdAt = entry.lastModified();

        readStashMeta(entry.absoluteFilePath(), info);

        // If fileCount was not set in meta, calculate from data directory
        if (info.fileCount == 0) {
            QString dataDir = QDir(info.folderPath).filePath("data");
            if (QDir(dataDir).exists()) {
                QDirIterator it(dataDir, QDir::Files, QDirIterator::Subdirectories);
                while (it.hasNext()) {
                    it.next();
                    info.fileCount++;
                    info.totalBytes += it.fileInfo().size();
                }
            }
        }

        result.append(info);
    }

    // Sort descending by creation date
    std::sort(result.begin(), result.end(), [](const StashInfo &a, const StashInfo &b) {
        return a.createdAt > b.createdAt;
    });

    return result;
}
