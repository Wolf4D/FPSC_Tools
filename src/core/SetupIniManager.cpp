#include "SetupIniManager.h"
#include "ProcessController.h"
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QFileInfo>
#include <QMap>
#include <QDebug>

SetupIniManager& SetupIniManager::instance()
{
    static SetupIniManager s_instance;
    return s_instance;
}

SetupIniManager::SetupIniManager(QObject *parent)
    : QObject(parent)
{
}

QString SetupIniManager::setupIniPath(const QString &fpscDir) const
{
    if (fpscDir.isEmpty()) return "";
    return QDir(fpscDir).filePath("setup.ini");
}

bool SetupIniManager::updateSetupIniFile(const QString &filePath, const LightmapProfile &profile)
{
    QFile file(filePath);
    if (!file.exists()) {
        // Create new setup.ini if it doesn't exist
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            return false;
        }
        QTextStream out(&file);
        out << "[SETUP]\n";
        out << "lightmapping=" << profile.lightmapping << "\n";
        out << "lightmaptexsize=" << profile.lightmaptexsize << "\n";
        out << "lightmapquality=" << profile.lightmapquality << "\n";
        out << "lightao=" << profile.lightao << "\n";
        file.close();
        return true;
    }

    // Read existing setup.ini
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }

    QStringList lines;
    QTextStream in(&file);
    while (!in.atEnd()) {
        lines.append(in.readLine());
    }
    file.close();

    bool foundLightmapping = false;
    bool foundLightmaptexsize = false;
    bool foundLightmapquality = false;
    bool foundLightao = false;

    QStringList updatedLines;

    for (const QString &line : lines) {
        QString trimmed = line.trimmed();
        if (trimmed.startsWith(";") || trimmed.startsWith("//")) {
            updatedLines.append(line);
            continue;
        }

        int eqIndex = trimmed.indexOf('=');
        if (eqIndex != -1) {
            QString key = trimmed.left(eqIndex).trimmed().toLower();
            if (key == "lightmapping") {
                updatedLines.append(QString("lightmapping=%1").arg(profile.lightmapping));
                foundLightmapping = true;
                continue;
            } else if (key == "lightmaptexsize") {
                updatedLines.append(QString("lightmaptexsize=%1").arg(profile.lightmaptexsize));
                foundLightmaptexsize = true;
                continue;
            } else if (key == "lightmapquality") {
                updatedLines.append(QString("lightmapquality=%1").arg(profile.lightmapquality));
                foundLightmapquality = true;
                continue;
            } else if (key == "lightao") {
                updatedLines.append(QString("lightao=%1").arg(profile.lightao));
                foundLightao = true;
                continue;
            }
        }

        updatedLines.append(line);
    }

    // Append any keys that were missing in setup.ini
    if (!foundLightmapping) {
        updatedLines.append(QString("lightmapping=%1").arg(profile.lightmapping));
    }
    if (!foundLightmaptexsize) {
        updatedLines.append(QString("lightmaptexsize=%1").arg(profile.lightmaptexsize));
    }
    if (!foundLightmapquality) {
        updatedLines.append(QString("lightmapquality=%1").arg(profile.lightmapquality));
    }
    if (!foundLightao) {
        updatedLines.append(QString("lightao=%1").arg(profile.lightao));
    }

    // Write back
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        return false;
    }

    QTextStream out(&file);
    for (const QString &l : updatedLines) {
        out << l << "\n";
    }
    file.close();

    return true;
}

bool SetupIniManager::applyProfile(const QString &fpscDir, const LightmapProfile &profile, bool restartProcess)
{
    QString iniFile = setupIniPath(fpscDir);
    if (iniFile.isEmpty()) {
        emit profileApplied(profile.id, false);
        return false;
    }

    // 1. Update setup.ini
    bool ok = updateSetupIniFile(iniFile, profile);
    if (!ok) {
        emit profileApplied(profile.id, false);
        return false;
    }

    // 2. Restart FPS Creator if requested
    if (restartProcess) {
        ProcessController::instance().restartFPSC(fpscDir);
    }

    emit profileApplied(profile.id, true);
    return true;
}

bool SetupIniManager::readCurrentValues(const QString &fpscDir, int &lightmapping, int &lightmaptexsize, int &lightmapquality, int &lightao)
{
    QString iniFile = setupIniPath(fpscDir);
    if (iniFile.isEmpty() || !QFile::exists(iniFile)) {
        return false;
    }

    QFile file(iniFile);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }

    lightmapping = -1;
    lightmaptexsize = -1;
    lightmapquality = -1;
    lightao = -1;

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.startsWith(";") || line.startsWith("//")) continue;

        int eq = line.indexOf('=');
        if (eq != -1) {
            QString key = line.left(eq).trimmed().toLower();
            QString val = line.mid(eq + 1).trimmed();
            if (key == "lightmapping") lightmapping = val.toInt();
            else if (key == "lightmaptexsize") lightmaptexsize = val.toInt();
            else if (key == "lightmapquality") lightmapquality = val.toInt();
            else if (key == "lightao") lightao = val.toInt();
        }
    }
    file.close();
    return true;
}
