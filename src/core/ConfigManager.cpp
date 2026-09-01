#include "ConfigManager.h"
#include "I18n.h"
#include <QCoreApplication>
#include <QGuiApplication>
#include <QScreen>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>

ConfigManager& ConfigManager::instance()
{
    static ConfigManager s_instance;
    return s_instance;
}

ConfigManager::ConfigManager(QObject *parent)
    : QObject(parent)
    , m_alwaysOnTop(true)
    , m_miniMode(false)
    , m_toolbarPos(-1, -1)
    , m_miniWidgetPos(-1, -1)
    , m_autoCleanOnSwitch(false)
    , m_showNotifications(true)
    , m_startMinimized(false)
{
    QString configPath = QCoreApplication::applicationDirPath() + "/fpsc_tool.ini";
    m_settings = new QSettings(configPath, QSettings::IniFormat, this);
    
    initDefaultProfiles();
    loadSettings();
}

ConfigManager::~ConfigManager()
{
    saveSettings();
}

void ConfigManager::initDefaultProfiles()
{
    m_profiles.clear();

    // 1. Disabled (Fastest)
    LightmapProfile disabled;
    disabled.id = "disabled";
    disabled.displayName = "Отключено";
    disabled.lightmapping = 0;
    disabled.lightmaptexsize = 16;
    disabled.lightmapquality = 16;
    disabled.lightao = 0;
    m_profiles.append(disabled);

    // 2. Fast
    LightmapProfile fast;
    fast.id = "fast";
    fast.displayName = "Быстрый";
    fast.lightmapping = 1;
    fast.lightmaptexsize = 128;
    fast.lightmapquality = 5;
    fast.lightao = 0;
    m_profiles.append(fast);

    // 3. Normal
    LightmapProfile normal;
    normal.id = "normal";
    normal.displayName = "Обычный";
    normal.lightmapping = 1;
    normal.lightmaptexsize = 256;
    normal.lightmapquality = 16;
    normal.lightao = 0;
    m_profiles.append(normal);

    // 4. Release
    LightmapProfile release;
    release.id = "release";
    release.displayName = "Релиз";
    release.lightmapping = 1;
    release.lightmaptexsize = 256;
    release.lightmapquality = 50;
    release.lightao = 1;
    m_profiles.append(release);

    // 5. Ultra
    LightmapProfile ultra;
    ultra.id = "ultra";
    ultra.displayName = "Ультра";
    ultra.lightmapping = 1;
    ultra.lightmaptexsize = 512;
    ultra.lightmapquality = 100;
    ultra.lightao = 1;
    m_profiles.append(ultra);

    m_activeProfileId = "normal";
}

void ConfigManager::loadSettings()
{
    m_fpscDirectory = m_settings->value("General/FpscDirectory", "").toString();
    if (m_fpscDirectory.isEmpty() || !isValidFpscDirectory(m_fpscDirectory)) {
        m_fpscDirectory = autoDetectFpscDirectory();
    }

    QString langCode = m_settings->value("General/Language", "auto").toString();
    I18n::instance().setLanguageSettingCode(langCode);

    m_alwaysOnTop = m_settings->value("UI/AlwaysOnTop", true).toBool();
    m_miniMode = m_settings->value("UI/MiniMode", false).toBool();
    m_toolbarPos = m_settings->value("UI/ToolbarPosition", QPoint(-1, -1)).toPoint();
    m_miniWidgetPos = m_settings->value("UI/MiniWidgetPosition", QPoint(-1, -1)).toPoint();
    m_autoCleanOnSwitch = m_settings->value("General/AutoCleanOnSwitch", false).toBool();
    m_showNotifications = m_settings->value("General/ShowNotifications", true).toBool();
    m_startMinimized = m_settings->value("General/StartMinimized", false).toBool();
    m_activeProfileId = m_settings->value("Profiles/ActiveProfileId", "normal").toString();

    int profileCount = m_settings->beginReadArray("CustomProfiles");
    if (profileCount > 0) {
        for (int i = 0; i < profileCount; ++i) {
            m_settings->setArrayIndex(i);
            QString id = m_settings->value("id").toString();
            for (LightmapProfile &p : m_profiles) {
                if (p.id == id) {
                    p.displayName = m_settings->value("displayName", p.displayName).toString();
                    p.lightmapping = m_settings->value("lightmapping", p.lightmapping).toInt();
                    p.lightmaptexsize = m_settings->value("lightmaptexsize", p.lightmaptexsize).toInt();
                    p.lightmapquality = m_settings->value("lightmapquality", p.lightmapquality).toInt();
                    p.lightao = m_settings->value("lightao", p.lightao).toInt();
                    break;
                }
            }
        }
    }
    m_settings->endArray();
}

void ConfigManager::saveSettings()
{
    m_settings->setValue("General/FpscDirectory", m_fpscDirectory);
    m_settings->setValue("General/Language", language());
    m_settings->setValue("UI/AlwaysOnTop", m_alwaysOnTop);
    m_settings->setValue("UI/MiniMode", m_miniMode);
    m_settings->setValue("UI/ToolbarPosition", m_toolbarPos);
    m_settings->setValue("UI/MiniWidgetPosition", m_miniWidgetPos);
    m_settings->setValue("General/AutoCleanOnSwitch", m_autoCleanOnSwitch);
    m_settings->setValue("General/ShowNotifications", m_showNotifications);
    m_settings->setValue("General/StartMinimized", m_startMinimized);
    m_settings->setValue("Profiles/ActiveProfileId", m_activeProfileId);

    m_settings->beginWriteArray("CustomProfiles", m_profiles.size());
    for (int i = 0; i < m_profiles.size(); ++i) {
        m_settings->setArrayIndex(i);
        const LightmapProfile &p = m_profiles[i];
        m_settings->setValue("id", p.id);
        m_settings->setValue("displayName", p.displayName);
        m_settings->setValue("lightmapping", p.lightmapping);
        m_settings->setValue("lightmaptexsize", p.lightmaptexsize);
        m_settings->setValue("lightmapquality", p.lightmapquality);
        m_settings->setValue("lightao", p.lightao);
    }
    m_settings->endArray();
    m_settings->sync();
}

QString ConfigManager::fpscDirectory() const
{
    return m_fpscDirectory;
}

void ConfigManager::setFpscDirectory(const QString &path)
{
    QString cleanPath = QDir::fromNativeSeparators(path).trimmed();
    if (cleanPath.endsWith('/')) {
        cleanPath.chop(1);
    }

    if (m_fpscDirectory != cleanPath) {
        m_fpscDirectory = cleanPath;
        saveSettings();
        emit fpscDirectoryChanged(m_fpscDirectory);
    }
}

bool ConfigManager::isValidFpscDirectory(const QString &path) const
{
    if (path.isEmpty()) return false;
    QDir dir(path);
    if (!dir.exists()) return false;

    // Check for standard FPSC files or Files/ directory
    bool hasFilesDir = dir.exists("Files");
    bool hasFpscExe = QFile::exists(dir.filePath("FPS Creator.exe")) || 
                      QFile::exists(dir.filePath("FPS Creator Mapeditor.exe")) ||
                      QFile::exists(dir.filePath("setup.ini"));

    return hasFilesDir || hasFpscExe;
}

QString ConfigManager::autoDetectFpscDirectory() const
{
    // Check 1: Current working dir
    QString current = QDir::currentPath();
    if (isValidFpscDirectory(current)) return current;

    // Check 2: Application dir
    QString appDir = QCoreApplication::applicationDirPath();
    if (isValidFpscDirectory(appDir)) return appDir;

    // Check 3: Parent of application dir (if inside Tools/ or similar)
    QDir parentDir(appDir);
    if (parentDir.cdUp() && isValidFpscDirectory(parentDir.absolutePath())) {
        return parentDir.absolutePath();
    }

    // Check 4: Common TGC install directories
    QStringList commonPaths = {
        "C:/Program Files (x86)/The Game Creators/FPS Creator",
        "C:/Program Files/The Game Creators/FPS Creator",
        "D:/Games/FPS Creator",
        "D:/FPS Creator",
        "C:/FPS Creator",
        "C:/FPSC",
        "D:/FPSC"
    };

    for (const QString &p : commonPaths) {
        if (isValidFpscDirectory(p)) {
            return p;
        }
    }

    return "";
}

QVector<LightmapProfile> ConfigManager::profiles() const
{
    QVector<LightmapProfile> result = m_profiles;
    for (LightmapProfile &p : result) {
        if (p.id == "disabled") p.displayName = trText("profile_disabled");
        else if (p.id == "fast") p.displayName = trText("profile_fast");
        else if (p.id == "normal") p.displayName = trText("profile_normal");
        else if (p.id == "release") p.displayName = trText("profile_release");
        else if (p.id == "ultra") p.displayName = trText("profile_ultra");
    }
    return result;
}

void ConfigManager::setProfiles(const QVector<LightmapProfile> &profiles)
{
    m_profiles = profiles;
    saveSettings();
    emit profilesChanged();
}

LightmapProfile ConfigManager::profileById(const QString &id) const
{
    QVector<LightmapProfile> current = profiles();
    for (const LightmapProfile &p : current) {
        if (p.id == id) return p;
    }
    if (!current.isEmpty()) return current.first();
    return LightmapProfile();
}

void ConfigManager::updateProfile(const LightmapProfile &profile)
{
    for (int i = 0; i < m_profiles.size(); ++i) {
        if (m_profiles[i].id == profile.id) {
            m_profiles[i] = profile;
            saveSettings();
            emit profilesChanged();
            return;
        }
    }
}

QString ConfigManager::activeProfileId() const
{
    return m_activeProfileId;
}

void ConfigManager::setActiveProfileId(const QString &id)
{
    if (m_activeProfileId != id) {
        m_activeProfileId = id;
        saveSettings();
        emit activeProfileChanged(m_activeProfileId);
    }
}

bool ConfigManager::isAlwaysOnTop() const
{
    return m_alwaysOnTop;
}

void ConfigManager::setAlwaysOnTop(bool onTop)
{
    if (m_alwaysOnTop != onTop) {
        m_alwaysOnTop = onTop;
        saveSettings();
        emit alwaysOnTopChanged(m_alwaysOnTop);
    }
}

bool ConfigManager::isMiniMode() const
{
    return m_miniMode;
}

void ConfigManager::setMiniMode(bool mini)
{
    m_miniMode = mini;
    saveSettings();
}

QPoint ConfigManager::defaultToolbarPosition(const QSize &windowSize) const
{
    QRect screenRect;
    QScreen *primary = QGuiApplication::primaryScreen();
    if (primary) {
        screenRect = primary->availableGeometry();
    } else {
        screenRect = QRect(0, 0, 1920, 1080);
    }

    int w = windowSize.width() > 0 ? windowSize.width() : 296;
    int h = windowSize.height() > 0 ? windowSize.height() : 82;

    int x = screenRect.right() - w - 16;
    int y = screenRect.bottom() - h - 16;
    return QPoint(x, y);
}

QPoint ConfigManager::defaultMiniWidgetPosition(const QSize &widgetSize) const
{
    QRect screenRect;
    QScreen *primary = QGuiApplication::primaryScreen();
    if (primary) {
        screenRect = primary->availableGeometry();
    } else {
        screenRect = QRect(0, 0, 1920, 1080);
    }

    int w = widgetSize.width() > 0 ? widgetSize.width() : 48;
    int h = widgetSize.height() > 0 ? widgetSize.height() : 48;

    int x = screenRect.right() - w - 24;
    int y = screenRect.bottom() - h - 24;
    return QPoint(x, y);
}

QPoint ConfigManager::toolbarPosition() const
{
    if (m_toolbarPos.x() < 0 || m_toolbarPos.y() < 0) {
        return defaultToolbarPosition();
    }
    return m_toolbarPos;
}

void ConfigManager::setToolbarPosition(const QPoint &pos)
{
    m_toolbarPos = pos;
    saveSettings();
}

QPoint ConfigManager::miniWidgetPosition() const
{
    if (m_miniWidgetPos.x() < 0 || m_miniWidgetPos.y() < 0) {
        return defaultMiniWidgetPosition();
    }
    return m_miniWidgetPos;
}

void ConfigManager::setMiniWidgetPosition(const QPoint &pos)
{
    m_miniWidgetPos = pos;
    saveSettings();
}

bool ConfigManager::autoCleanOnSwitch() const
{
    return m_autoCleanOnSwitch;
}

void ConfigManager::setAutoCleanOnSwitch(bool enable)
{
    m_autoCleanOnSwitch = enable;
    saveSettings();
}

bool ConfigManager::showNotifications() const
{
    return m_showNotifications;
}

void ConfigManager::setShowNotifications(bool enable)
{
    m_showNotifications = enable;
    saveSettings();
}

bool ConfigManager::startMinimized() const
{
    return m_startMinimized;
}

void ConfigManager::setStartMinimized(bool enable)
{
    m_startMinimized = enable;
    saveSettings();
}

QString ConfigManager::language() const
{
    return I18n::instance().languageSettingCode();
}

void ConfigManager::setLanguage(const QString &lang)
{
    if (language() != lang) {
        I18n::instance().setLanguageSettingCode(lang);
        saveSettings();
        emit languageChanged();
        emit profilesChanged();
    }
}
