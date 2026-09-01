#include "TrayManager.h"
#include "ConfigManager.h"
#include "ProcessController.h"
#include "Theme.h"
#include "I18n.h"
#include <QAction>
#include <QActionGroup>
#include <QPainter>
#include <QPixmap>
#include <QApplication>

TrayManager::TrayManager(QObject *parent)
    : QObject(parent)
    , m_trayIcon(nullptr)
    , m_trayMenu(nullptr)
{
}

QIcon TrayManager::createTrayIcon(bool isFpscRunning)
{
    QPixmap pixmap(32, 32);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    // Favicon image filling full icon area
    QPixmap iconPix(":/app.png");
    if (iconPix.isNull()) iconPix = QPixmap(":/app.ico");
    if (!iconPix.isNull()) {
        painter.drawPixmap(0, 0, 32, 32, iconPix);
    }

    // Status dot (Bottom Right badge with dark outline)
    QColor dot = isFpscRunning ? QColor(0, 230, 118) : QColor(255, 82, 82);
    painter.setBrush(dot);
    painter.setPen(QPen(QColor(18, 20, 24), 2.0));
    painter.drawEllipse(QPoint(25, 25), 4, 4);

    return QIcon(pixmap);
}

void TrayManager::initTray()
{
    if (m_trayIcon) return;

    m_trayIcon = new QSystemTrayIcon(this);
    m_trayIcon->setIcon(createTrayIcon(ProcessController::instance().isFPSCRunning()));
    m_trayIcon->setToolTip("FPSC Tool");

    createMenu();

    connect(m_trayIcon, &QSystemTrayIcon::activated, this, &TrayManager::onTrayActivated);
    connect(&ProcessController::instance(), &ProcessController::fpscStatusChanged, this, [this](bool running) {
        m_trayIcon->setIcon(createTrayIcon(running));
    });

    m_trayIcon->show();
}

void TrayManager::createMenu()
{
    if (m_trayMenu) {
        delete m_trayMenu;
    }

    m_trayMenu = new QMenu();
    m_trayMenu->setStyleSheet(Theme::darkStyleSheet());

    // 1. Show/Hide toolbar
    QAction *toggleAct = m_trayMenu->addAction(trText("menu_toggle_toolbar"));
    connect(toggleAct, &QAction::triggered, this, &TrayManager::toggleToolbarRequested);

    m_trayMenu->addSeparator();

    // 2. Profiles Submenu
    QMenu *profilesMenu = m_trayMenu->addMenu(trText("menu_profiles"));
    QActionGroup *group = new QActionGroup(profilesMenu);

    QVector<LightmapProfile> profiles = ConfigManager::instance().profiles();
    QString activeId = ConfigManager::instance().activeProfileId();

    for (const LightmapProfile &p : profiles) {
        QAction *act = profilesMenu->addAction(p.displayName);
        act->setCheckable(true);
        act->setChecked(p.id == activeId);
        group->addAction(act);
        connect(act, &QAction::triggered, this, [this, p]() {
            emit profileSelected(p.id);
        });
    }

    // NOTE: Manual LightMapper execution commented out - functionality does not work completely
    /*
    profilesMenu->addSeparator();
    QAction *runLmAct = profilesMenu->addAction(trText("menu_run_lightmapper"));
    connect(runLmAct, &QAction::triggered, this, &TrayManager::runLightMapperRequested);
    */

    m_trayMenu->addSeparator();

    // 3. Process Actions
    QAction *restartAct = m_trayMenu->addAction(trText("menu_restart"));
    connect(restartAct, &QAction::triggered, this, &TrayManager::restartRequested);

    QAction *killAct = m_trayMenu->addAction(trText("menu_kill"));
    connect(killAct, &QAction::triggered, this, &TrayManager::killRequested);

    m_trayMenu->addSeparator();

    // 4. Clean Submenu
    QMenu *cleanMenu = m_trayMenu->addMenu(trText("menu_clean"));
    QAction *cleanBin = cleanMenu->addAction(trText("clean_bin_dbo"));
    connect(cleanBin, &QAction::triggered, this, [this]() { emit cleanRequested(0); });

    QAction *cleanLevel = cleanMenu->addAction(trText("clean_level_data"));
    connect(cleanLevel, &QAction::triggered, this, [this]() { emit cleanRequested(3); });

    QAction *cleanAll = cleanMenu->addAction(trText("clean_all"));
    connect(cleanAll, &QAction::triggered, this, [this]() { emit cleanRequested(2); });

    cleanMenu->addSeparator();

    QAction *cleanTemp = cleanMenu->addAction(trText("clean_engine_temp"));
    connect(cleanTemp, &QAction::triggered, this, [this]() { emit cleanRequested(4); });

    // 5. Stash Submenu
    QMenu *stashMenu = m_trayMenu->addMenu(trText("menu_stash"));
    QAction *saveStash = stashMenu->addAction(trText("stash_save_quick"));
    connect(saveStash, &QAction::triggered, this, &TrayManager::stashSaveRequested);

    QAction *restoreStash = stashMenu->addAction(trText("stash_restore_quick"));
    connect(restoreStash, &QAction::triggered, this, &TrayManager::stashRestoreRequested);

    QAction *manageStash = stashMenu->addAction(trText("stash_manager"));
    connect(manageStash, &QAction::triggered, this, &TrayManager::stashManagerRequested);

    m_trayMenu->addSeparator();

    // 6. Settings & Exit
    QAction *settingsAct = m_trayMenu->addAction(trText("menu_settings"));
    connect(settingsAct, &QAction::triggered, this, &TrayManager::settingsRequested);

    m_trayMenu->addSeparator();

    QAction *exitAct = m_trayMenu->addAction(trText("menu_exit"));
    connect(exitAct, &QAction::triggered, this, &TrayManager::exitRequested);

    m_trayIcon->setContextMenu(m_trayMenu);
}

void TrayManager::updateMenu()
{
    createMenu();
}

void TrayManager::showNotification(const QString &title, const QString &message, QSystemTrayIcon::MessageIcon icon)
{
    if (ConfigManager::instance().showNotifications() && m_trayIcon && m_trayIcon->isVisible()) {
        m_trayIcon->showMessage(title, message, icon, 3000);
    }
}

void TrayManager::onTrayActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::Trigger || reason == QSystemTrayIcon::DoubleClick) {
        emit toggleToolbarRequested();
    }
}
