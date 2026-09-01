#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <QString>
#include <QVector>
#include <QSettings>
#include <QPoint>
#include <QSize>
#include <QObject>

struct LightmapProfile {
    QString id;
    QString displayName;
    int lightmapping;       // 0 or 1
    int lightmaptexsize;    // e.g. 512, 256, 128, 16
    int lightmapquality;    // e.g. 100, 50, 16, 5, 16
    int lightao;            // 0 or 1

    bool operator==(const LightmapProfile &other) const {
        return id == other.id &&
               lightmapping == other.lightmapping &&
               lightmaptexsize == other.lightmaptexsize &&
               lightmapquality == other.lightmapquality &&
               lightao == other.lightao;
    }
};

class ConfigManager : public QObject
{
    Q_OBJECT

public:
    static ConfigManager& instance();

    void loadSettings();
    void saveSettings();

    QString fpscDirectory() const;
    void setFpscDirectory(const QString &path);
    bool isValidFpscDirectory(const QString &path) const;
    QString autoDetectFpscDirectory() const;

    QVector<LightmapProfile> profiles() const;
    void setProfiles(const QVector<LightmapProfile> &profiles);
    LightmapProfile profileById(const QString &id) const;
    void updateProfile(const LightmapProfile &profile);

    QString activeProfileId() const;
    void setActiveProfileId(const QString &id);

    bool isAlwaysOnTop() const;
    void setAlwaysOnTop(bool onTop);

    bool isMiniMode() const;
    void setMiniMode(bool mini);

    QPoint toolbarPosition() const;
    void setToolbarPosition(const QPoint &pos);
    QPoint defaultToolbarPosition(const QSize &windowSize = QSize(296, 82)) const;

    QPoint miniWidgetPosition() const;
    void setMiniWidgetPosition(const QPoint &pos);
    QPoint defaultMiniWidgetPosition(const QSize &widgetSize = QSize(48, 48)) const;

    bool autoCleanOnSwitch() const;
    void setAutoCleanOnSwitch(bool enable);

    bool showNotifications() const;
    void setShowNotifications(bool enable);

    bool startMinimized() const;
    void setStartMinimized(bool enable);

    QString language() const;
    void setLanguage(const QString &lang);

signals:
    void fpscDirectoryChanged(const QString &path);
    void activeProfileChanged(const QString &id);
    void profilesChanged();
    void alwaysOnTopChanged(bool onTop);
    void languageChanged();

private:
    explicit ConfigManager(QObject *parent = nullptr);
    ~ConfigManager() override;
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;

    void initDefaultProfiles();

    QString m_fpscDirectory;
    QVector<LightmapProfile> m_profiles;
    QString m_activeProfileId;
    bool m_alwaysOnTop;
    bool m_miniMode;
    QPoint m_toolbarPos;
    QPoint m_miniWidgetPos;
    bool m_autoCleanOnSwitch;
    bool m_showNotifications;
    bool m_startMinimized;

    QSettings *m_settings;
};

#endif // CONFIGMANAGER_H
