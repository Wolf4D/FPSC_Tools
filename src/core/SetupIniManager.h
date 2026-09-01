#ifndef SETUPINIMANAGER_H
#define SETUPINIMANAGER_H

#include <QObject>
#include <QString>
#include "ConfigManager.h"

class SetupIniManager : public QObject
{
    Q_OBJECT

public:
    static SetupIniManager& instance();

    bool applyProfile(const QString &fpscDir, const LightmapProfile &profile, bool restartProcess = true);
    bool readCurrentValues(const QString &fpscDir, int &lightmapping, int &lightmaptexsize, int &lightmapquality, int &lightao);
    QString setupIniPath(const QString &fpscDir) const;

signals:
    void profileApplied(const QString &profileId, bool success);

private:
    explicit SetupIniManager(QObject *parent = nullptr);
    ~SetupIniManager() override = default;
    SetupIniManager(const SetupIniManager&) = delete;
    SetupIniManager& operator=(const SetupIniManager&) = delete;

    bool updateSetupIniFile(const QString &filePath, const LightmapProfile &profile);
};

#endif // SETUPINIMANAGER_H
