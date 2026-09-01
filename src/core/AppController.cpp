#include "AppController.h"
#include "ConfigManager.h"
#include "ProcessController.h"
#include "SetupIniManager.h"
#include "FastCleaner.h"
#include "StashManager.h"
#include "SettingsDialog.h"
#include "StashDialog.h"
#include "I18n.h"
#include <QApplication>
#include <QGuiApplication>
#include <QScreen>
#include <QMessageBox>

AppController::AppController(QObject *parent)
    : QObject(parent)
    , m_toolbar(nullptr)
    , m_miniWidget(nullptr)
    , m_trayManager(nullptr)
    , m_foldOffsetFromToolbar(240, 12)
{
}

AppController::~AppController()
{
    if (m_toolbar) {
        ConfigManager::instance().setToolbarPosition(m_toolbar->pos());
    }
    if (m_miniWidget) {
        ConfigManager::instance().setMiniWidgetPosition(m_miniWidget->pos());
    }
}

void AppController::start()
{
    ConfigManager &cfg = ConfigManager::instance();

    m_toolbar = new FloatingToolbar();
    m_miniWidget = new MiniIconWidget();
    m_trayManager = new TrayManager(this);

    m_toolbar->move(cfg.toolbarPosition());
    m_miniWidget->move(cfg.miniWidgetPosition());

    m_trayManager->initTray();
    setupConnections();

    // Check if FPSC directory is configured, if not, prompt once
    if (cfg.fpscDirectory().isEmpty() || !cfg.isValidFpscDirectory(cfg.fpscDirectory())) {
        QString detected = cfg.autoDetectFpscDirectory();
        if (!detected.isEmpty()) {
            cfg.setFpscDirectory(detected);
        }
    }

    // Decide initial visibility
    if (cfg.startMinimized()) {
        hideAllToTray();
    } else if (cfg.isMiniMode()) {
        showMiniIconWidget();
    } else {
        showFloatingToolbar();
    }
}

void AppController::setupConnections()
{
    // Toolbar events
    connect(m_toolbar, &FloatingToolbar::foldRequested, this, &AppController::showMiniIconWidget);
    connect(m_toolbar, &FloatingToolbar::minimizeToTrayRequested, this, &AppController::hideAllToTray);
    connect(m_toolbar, &FloatingToolbar::showStatusMessage, this, &AppController::onStatusMessage);

    // Mini widget events
    connect(m_miniWidget, &MiniIconWidget::expandRequested, this, &AppController::showFloatingToolbar);
    connect(m_miniWidget, &MiniIconWidget::restartRequested, this, &AppController::onRestartRequested);
    connect(m_miniWidget, &MiniIconWidget::killRequested, this, &AppController::onKillRequested);
    connect(m_miniWidget, &MiniIconWidget::cleanRequested, this, &AppController::onCleanRequested);
    connect(m_miniWidget, &MiniIconWidget::stashSaveRequested, this, &AppController::onStashSaveRequested);
    connect(m_miniWidget, &MiniIconWidget::stashRestoreRequested, this, &AppController::onStashRestoreRequested);
    connect(m_miniWidget, &MiniIconWidget::settingsRequested, this, &AppController::onSettingsRequested);
    connect(m_miniWidget, &MiniIconWidget::exitRequested, this, &AppController::onExitRequested);
    connect(m_miniWidget, &MiniIconWidget::profileSelected, this, &AppController::onProfileSelected);
    connect(m_miniWidget, &MiniIconWidget::runLightMapperRequested, m_toolbar, &FloatingToolbar::runLightMapper);

    // Tray manager events
    connect(m_trayManager, &TrayManager::toggleToolbarRequested, this, &AppController::toggleToolbarVisibility);
    connect(m_trayManager, &TrayManager::showToolbarRequested, this, &AppController::showFloatingToolbar);
    connect(m_trayManager, &TrayManager::profileSelected, this, &AppController::onProfileSelected);
    connect(m_trayManager, &TrayManager::runLightMapperRequested, m_toolbar, &FloatingToolbar::runLightMapper);
    connect(m_trayManager, &TrayManager::restartRequested, this, &AppController::onRestartRequested);
    connect(m_trayManager, &TrayManager::killRequested, this, &AppController::onKillRequested);
    connect(m_trayManager, &TrayManager::cleanRequested, this, &AppController::onCleanRequested);
    connect(m_trayManager, &TrayManager::stashSaveRequested, this, &AppController::onStashSaveRequested);
    connect(m_trayManager, &TrayManager::stashRestoreRequested, this, &AppController::onStashRestoreRequested);
    connect(m_trayManager, &TrayManager::stashManagerRequested, this, &AppController::onStashManagerRequested);
    connect(m_trayManager, &TrayManager::settingsRequested, this, &AppController::onSettingsRequested);
    connect(m_trayManager, &TrayManager::exitRequested, this, &AppController::onExitRequested);

    // Process status updates
    connect(&ProcessController::instance(), &ProcessController::fpscStatusChanged, this, [this](bool running) {
        LightmapProfile p = ConfigManager::instance().profileById(ConfigManager::instance().activeProfileId());
        m_miniWidget->updateStatus(running, p.displayName);
        m_toolbar->updateStatusIndicator();
    });

    // Profile updates
    connect(&ConfigManager::instance(), &ConfigManager::profilesChanged, this, [this]() {
        m_toolbar->updateProfileButton();
        m_trayManager->updateMenu();
    });

    connect(&ConfigManager::instance(), &ConfigManager::activeProfileChanged, this, [this](const QString &id) {
        m_toolbar->updateProfileButton();
        m_trayManager->updateMenu();
        LightmapProfile p = ConfigManager::instance().profileById(id);
        m_miniWidget->updateStatus(ProcessController::instance().isFPSCRunning(), p.displayName);
    });
}

void AppController::showFloatingToolbar()
{
    ConfigManager::instance().setMiniMode(false);

    QPoint targetPos;
    if (m_miniWidget && m_miniWidget->isVisible()) {
        // Expand such that the toolbar returns to the exact location relative to the mini icon center
        QPoint miniCenter(m_miniWidget->x() + m_miniWidget->width() / 2,
                          m_miniWidget->y() + m_miniWidget->height() / 2);
        targetPos = miniCenter - m_foldOffsetFromToolbar;
    } else {
        targetPos = ConfigManager::instance().toolbarPosition();
    }

    // Clamp targetPos to screen
    QScreen *screen = QGuiApplication::screenAt(targetPos);
    if (!screen) screen = QGuiApplication::primaryScreen();
    if (screen) {
        QRect avail = screen->availableGeometry();
        int x = qBound(avail.left(), targetPos.x(), avail.right() - m_toolbar->width());
        int y = qBound(avail.top(), targetPos.y(), avail.bottom() - m_toolbar->height());
        targetPos = QPoint(x, y);
    }

    m_miniWidget->hide();

    m_toolbar->move(targetPos);
    ConfigManager::instance().setToolbarPosition(targetPos);
    m_toolbar->show();
    m_toolbar->raise();
    m_toolbar->activateWindow();
}

void AppController::showMiniIconWidget()
{
    ConfigManager::instance().setMiniMode(true);

    QPoint targetPos;
    if (m_toolbar && m_toolbar->isVisible()) {
        ConfigManager::instance().setToolbarPosition(m_toolbar->pos());
        QPoint cursorPos = QCursor::pos();
        if (m_toolbar->frameGeometry().contains(cursorPos)) {
            // Place mini icon widget centered directly where the mouse click happened
            m_foldOffsetFromToolbar = cursorPos - m_toolbar->pos();
            targetPos = QPoint(cursorPos.x() - m_miniWidget->width() / 2,
                               cursorPos.y() - m_miniWidget->height() / 2);
        } else {
            m_foldOffsetFromToolbar = QPoint(m_toolbar->width() / 2, m_toolbar->height() / 2);
            targetPos = QPoint(m_toolbar->x() + m_toolbar->width() / 2 - m_miniWidget->width() / 2,
                               m_toolbar->y() + m_toolbar->height() / 2 - m_miniWidget->height() / 2);
        }
    } else {
        targetPos = ConfigManager::instance().miniWidgetPosition();
    }

    // Clamp targetPos to screen
    QScreen *screen = QGuiApplication::screenAt(targetPos);
    if (!screen) screen = QGuiApplication::primaryScreen();
    if (screen) {
        QRect avail = screen->availableGeometry();
        int x = qBound(avail.left(), targetPos.x(), avail.right() - m_miniWidget->width());
        int y = qBound(avail.top(), targetPos.y(), avail.bottom() - m_miniWidget->height());
        targetPos = QPoint(x, y);
    }

    m_toolbar->hide();

    LightmapProfile p = ConfigManager::instance().profileById(ConfigManager::instance().activeProfileId());
    m_miniWidget->updateStatus(ProcessController::instance().isFPSCRunning(), p.displayName);
    m_miniWidget->move(targetPos);
    ConfigManager::instance().setMiniWidgetPosition(targetPos);
    m_miniWidget->show();
    m_miniWidget->raise();
    m_miniWidget->activateWindow();
}

void AppController::toggleToolbarVisibility()
{
    if (m_toolbar->isVisible()) {
        hideAllToTray();
    } else if (m_miniWidget->isVisible()) {
        showFloatingToolbar();
    } else {
        showFloatingToolbar();
    }
}

void AppController::hideAllToTray()
{
    if (m_toolbar->isVisible()) {
        ConfigManager::instance().setToolbarPosition(m_toolbar->pos());
    }
    if (m_miniWidget->isVisible()) {
        ConfigManager::instance().setMiniWidgetPosition(m_miniWidget->pos());
    }

    m_toolbar->hide();
    m_miniWidget->hide();
    onStatusMessage(trText("msg_minimized_tray"));
}

void AppController::onProfileSelected(const QString &profileId)
{
    m_toolbar->switchProfile(profileId);
}

void AppController::onRestartRequested()
{
    m_toolbar->restartFpsc();
}

void AppController::onKillRequested()
{
    m_toolbar->killFpsc();
}

void AppController::onCleanRequested(int type)
{
    m_toolbar->cleanCache(static_cast<CleanType>(type));
}

void AppController::onStashSaveRequested()
{
    m_toolbar->quickStashSave();
}

void AppController::onStashRestoreRequested()
{
    m_toolbar->quickStashRestore();
}

void AppController::onStashManagerRequested()
{
    m_toolbar->openStashManager();
}

void AppController::onSettingsRequested()
{
    m_toolbar->openSettings();
}

void AppController::onExitRequested()
{
    qApp->quit();
}

void AppController::onStatusMessage(const QString &msg)
{
    if (m_trayManager) {
        m_trayManager->showNotification("FPSC Tool", msg);
    }
}
