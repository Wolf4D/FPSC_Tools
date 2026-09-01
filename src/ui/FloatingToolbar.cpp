#include "FloatingToolbar.h"
#include "ProcessController.h"
#include "SetupIniManager.h"
#include "StashManager.h"
#include "SettingsDialog.h"
#include "StashDialog.h"
#include "LightMapperProgressDialog.h"
#include "Theme.h"
#include "I18n.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QMouseEvent>
#include <QApplication>
#include <QMessageBox>
#include <QFrame>
#include <QStyle>
#include <QActionGroup>
#include <QDesktopServices>
#include <QUrl>
#include <QDir>
#include <QPainter>

FloatingToolbar::FloatingToolbar(QWidget *parent)
    : QWidget(parent, Qt::FramelessWindowHint | Qt::Tool)
    , m_isDragging(false)
    , m_profileMenu(nullptr)
    , m_cleanMenu(nullptr)
    , m_stashMenu(nullptr)
{
    setWindowTitle("FPSC Tools v1.0.0 beta");
    setAttribute(Qt::WA_TranslucentBackground, true);
    setStyleSheet(Theme::darkStyleSheet());
    setObjectName("toolbarContainer");

    setupUi();

    applyAlwaysOnTopFlag(ConfigManager::instance().isAlwaysOnTop());

    // Timer to periodically poll FPSC process status for LED indicator
    m_statusTimer = new QTimer(this);
    connect(m_statusTimer, &QTimer::timeout, this, &FloatingToolbar::updateStatusIndicator);
    m_statusTimer->start(1500);

    // Smooth hover tracker for zero-jitter auto-hiding header
    m_hoverTracker = new QTimer(this);
    connect(m_hoverTracker, &QTimer::timeout, this, &FloatingToolbar::checkHoverState);
    m_hoverTracker->start(40);

    connect(&ConfigManager::instance(), &ConfigManager::languageChanged, this, &FloatingToolbar::retranslateUi);

    updateStatusIndicator();
    updateProfileButton();
}

void FloatingToolbar::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(2);

    // --- 1. TOP TITLE / DRAG BAR (Appears on Hover) ---
    m_topBarWidget = new QWidget(this);
    m_topBarWidget->setObjectName("topBarCard");
    m_topBarWidget->setFixedHeight(24);

    QHBoxLayout *topBar = new QHBoxLayout(m_topBarWidget);
    topBar->setContentsMargins(6, 2, 4, 2);
    topBar->setSpacing(3);

    m_titleGripLabel = new QLabel("⋮⋮ FPSC Tools v1.0.0 beta", m_topBarWidget);
    m_titleGripLabel->setStyleSheet("font-size: 10px; font-weight: bold; color: #758195; letter-spacing: 0.5px; background: transparent; border: none;");
    m_titleGripLabel->setCursor(Qt::SizeAllCursor);
    m_titleGripLabel->setToolTip(trText("toolbar_title"));
    topBar->addWidget(m_titleGripLabel);

    topBar->addStretch();

    // Pin Button (Always On Top)
    m_pinBtn = new QPushButton("📌", m_topBarWidget);
    m_pinBtn->setObjectName("pinBtn");
    m_pinBtn->setProperty("class", "iconBtn");
    m_pinBtn->setFixedSize(18, 18);
    m_pinBtn->setProperty("pinned", ConfigManager::instance().isAlwaysOnTop());
    m_pinBtn->setToolTip(trText("pin_tooltip"));
    connect(m_pinBtn, &QPushButton::clicked, this, &FloatingToolbar::toggleAlwaysOnTop);
    topBar->addWidget(m_pinBtn);

    // Fold to Mini Icon
    m_foldBtn = new QPushButton("🗕", m_topBarWidget);
    m_foldBtn->setProperty("class", "iconBtn");
    m_foldBtn->setFixedSize(18, 18);
    m_foldBtn->setToolTip(trText("fold_tooltip"));
    connect(m_foldBtn, &QPushButton::clicked, this, &FloatingToolbar::foldToMiniIcon);
    topBar->addWidget(m_foldBtn);

    // Settings
    m_settingsBtn = new QPushButton("⚙", m_topBarWidget);
    m_settingsBtn->setProperty("class", "iconBtn");
    m_settingsBtn->setFixedSize(18, 18);
    m_settingsBtn->setToolTip(trText("settings_tooltip"));
    connect(m_settingsBtn, &QPushButton::clicked, this, &FloatingToolbar::openSettings);
    topBar->addWidget(m_settingsBtn);

    // Close (Exit)
    m_closeBtn = new QPushButton("✕", m_topBarWidget);
    m_closeBtn->setObjectName("closeBtn");
    m_closeBtn->setProperty("class", "iconBtn");
    m_closeBtn->setFixedSize(18, 18);
    m_closeBtn->setToolTip(trText("close_tooltip"));
    connect(m_closeBtn, &QPushButton::clicked, qApp, &QApplication::quit);
    topBar->addWidget(m_closeBtn);

    mainLayout->addWidget(m_topBarWidget);
    m_topBarWidget->hide();

    // --- 2. MAIN CONTENT ROW (Icon Button + Actions Grid) ---
    m_contentWidget = new QWidget(this);
    m_contentWidget->setObjectName("contentCard");

    QHBoxLayout *contentLayout = new QHBoxLayout(m_contentWidget);
    contentLayout->setContentsMargins(6, 4, 6, 4);
    contentLayout->setSpacing(5);

    // Left Section: Full-height Favicon Button with Status LED overlay
    m_statusIconBtn = new QPushButton(m_contentWidget);
    m_statusIconBtn->setObjectName("statusIconBtn");
    m_statusIconBtn->setFixedSize(48, 48);
    m_statusIconBtn->setIconSize(QSize(48, 48));
    m_statusIconBtn->setCursor(Qt::PointingHandCursor);
    m_statusIconBtn->setStyleSheet("background: transparent; border: none; padding: 0px;");
    connect(m_statusIconBtn, &QPushButton::clicked, this, &FloatingToolbar::restartFpsc);
    contentLayout->addWidget(m_statusIconBtn, 0, Qt::AlignVCenter);

    // Vertical Separator
    QFrame *vSep = new QFrame(m_contentWidget);
    vSep->setFrameShape(QFrame::VLine);
    vSep->setStyleSheet("color: #383e4d;");
    contentLayout->addWidget(vSep);

    // Center Section: 2x2 Grid of Actions
    QGridLayout *actionsGrid = new QGridLayout();
    actionsGrid->setContentsMargins(0, 0, 0, 0);
    actionsGrid->setHorizontalSpacing(4);
    actionsGrid->setVerticalSpacing(3);

    // Row 0, Col 0: Profile Dropdown
    m_profileBtn = new QPushButton(m_contentWidget);
    m_profileBtn->setObjectName("profileBtn");
    m_profileBtn->setFixedWidth(112);
    actionsGrid->addWidget(m_profileBtn, 0, 0);

    // Row 0, Col 1: Restart Button
    m_restartBtn = new QPushButton(trText("restart_btn"), m_contentWidget);
    m_restartBtn->setObjectName("restartBtn");
    m_restartBtn->setFixedWidth(112);
    m_restartBtn->setToolTip(trText("restart_tooltip"));
    connect(m_restartBtn, &QPushButton::clicked, this, &FloatingToolbar::restartFpsc);
    actionsGrid->addWidget(m_restartBtn, 0, 1);

    // Row 1, Col 0: Clean Dropdown
    m_cleanBtn = new QPushButton(trText("clean_btn"), m_contentWidget);
    m_cleanBtn->setObjectName("cleanBtn");
    m_cleanBtn->setFixedWidth(112);
    m_cleanBtn->setToolTip(trText("clean_tooltip"));
    actionsGrid->addWidget(m_cleanBtn, 1, 0);

    // Row 1, Col 1: Stash Dropdown
    m_stashBtn = new QPushButton(trText("stash_btn"), m_contentWidget);
    m_stashBtn->setObjectName("stashBtn");
    m_stashBtn->setFixedWidth(112);
    m_stashBtn->setToolTip(trText("stash_tooltip"));
    actionsGrid->addWidget(m_stashBtn, 1, 1);

    contentLayout->addLayout(actionsGrid);

    mainLayout->addWidget(m_contentWidget);

    setFixedSize(316, 82);

    rebuildProfileMenu();
    rebuildCleanMenu();
    rebuildStashMenu();
}

void FloatingToolbar::rebuildCleanMenu()
{
    if (m_cleanMenu) delete m_cleanMenu;
    m_cleanMenu = new QMenu(this);

    // 1. Clean .bin and .dbo
    QAction *actCleanBin = m_cleanMenu->addAction(trText("clean_bin_dbo"));
    connect(actCleanBin, &QAction::triggered, this, [this]() { cleanCache(CleanType::BinAndDbo); });

    // 2. Clean Level Build Data (.dbo, .ele, .lgt, .dat)
    QAction *actCleanLevel = m_cleanMenu->addAction(trText("clean_level_data"));
    connect(actCleanLevel, &QAction::triggered, this, [this]() { cleanCache(CleanType::LevelBuildData); });

    // 3. Clean All (.bin/.dbo + Level Data)
    QAction *actCleanAll = m_cleanMenu->addAction(trText("clean_all"));
    connect(actCleanAll, &QAction::triggered, this, [this]() { cleanCache(CleanType::All); });

    m_cleanMenu->addSeparator();

    // 4. Clean %TEMP% Engine Cache
    QAction *actCleanTemp = m_cleanMenu->addAction(trText("clean_engine_temp"));
    connect(actCleanTemp, &QAction::triggered, this, [this]() { cleanCache(CleanType::EngineTemp); });

    m_cleanBtn->setMenu(m_cleanMenu);
}

void FloatingToolbar::rebuildStashMenu()
{
    if (m_stashMenu) delete m_stashMenu;
    m_stashMenu = new QMenu(this);

    QAction *actStashSave = m_stashMenu->addAction(trText("stash_save_quick"));
    connect(actStashSave, &QAction::triggered, this, &FloatingToolbar::quickStashSave);

    QAction *actStashRestore = m_stashMenu->addAction(trText("stash_restore_quick"));
    connect(actStashRestore, &QAction::triggered, this, &FloatingToolbar::quickStashRestore);

    m_stashMenu->addSeparator();

    QAction *actStashManager = m_stashMenu->addAction(trText("stash_manager"));
    connect(actStashManager, &QAction::triggered, this, &FloatingToolbar::openStashManager);

    QAction *actStashFolder = m_stashMenu->addAction(trText("stash_folder"));
    connect(actStashFolder, &QAction::triggered, this, &FloatingToolbar::openStashFolder);

    m_stashBtn->setMenu(m_stashMenu);
}

void FloatingToolbar::retranslateUi()
{
    m_titleGripLabel->setToolTip(trText("toolbar_title"));
    m_restartBtn->setText(trText("restart_btn"));
    m_restartBtn->setToolTip(trText("restart_tooltip"));
    m_pinBtn->setToolTip(trText("pin_tooltip"));
    m_foldBtn->setToolTip(trText("fold_tooltip"));
    m_settingsBtn->setToolTip(trText("settings_tooltip"));
    m_closeBtn->setToolTip(trText("close_tooltip"));

    m_cleanBtn->setText(trText("clean_btn"));
    m_cleanBtn->setToolTip(trText("clean_tooltip"));

    m_stashBtn->setText(trText("stash_btn"));
    m_stashBtn->setToolTip(trText("stash_tooltip"));

    rebuildProfileMenu();
    rebuildCleanMenu();
    rebuildStashMenu();
    updateStatusIndicator();
}

void FloatingToolbar::rebuildProfileMenu()
{
    if (m_profileMenu) {
        delete m_profileMenu;
    }

    m_profileMenu = new QMenu(this);
    QActionGroup *group = new QActionGroup(m_profileMenu);

    QVector<LightmapProfile> profiles = ConfigManager::instance().profiles();
    QString activeId = ConfigManager::instance().activeProfileId();

    for (const LightmapProfile &p : profiles) {
        QString itemText = QString("%1  (tex: %2, qual: %3, ao: %4)")
            .arg(p.displayName)
            .arg(p.lightmaptexsize)
            .arg(p.lightmapquality)
            .arg(p.lightao);

        QAction *act = m_profileMenu->addAction(itemText);
        act->setCheckable(true);
        act->setChecked(p.id == activeId);
        group->addAction(act);

        connect(act, &QAction::triggered, this, [this, p]() {
            switchProfile(p.id);
        });
    }

    // NOTE: Manual LightMapper execution commented out - functionality does not work completely
    /*
    m_profileMenu->addSeparator();

    QAction *actRunLm = m_profileMenu->addAction(trText("menu_run_lightmapper"));
    connect(actRunLm, &QAction::triggered, this, &FloatingToolbar::runLightMapper);
    */

    m_profileBtn->setMenu(m_profileMenu);
    updateProfileButton();
}

void FloatingToolbar::runLightMapper()
{
    ConfigManager &cfg = ConfigManager::instance();
    QString fpscDir = cfg.fpscDirectory();

    if (fpscDir.isEmpty() || !cfg.isValidFpscDirectory(fpscDir)) {
        QMessageBox::warning(this, "FPS Creator", trText("msg_folder_not_set"));
        openSettings();
        return;
    }

    QString lmExe = ProcessController::instance().findLightMapperExe(fpscDir);
    if (lmExe.isEmpty()) {
        QMessageBox::warning(this, "LightMapper", trText("err_lightmapper_not_found"));
        return;
    }

    // Working directory is strictly the engine root directory
    QString workDir = fpscDir;

    LightMapperProgressDialog dlg(this);
    int exitCode = dlg.runProcess(QDir::toNativeSeparators(lmExe), QStringList() << "BIMlightmapper", QDir::toNativeSeparators(workDir));

    if (exitCode == 0) {
        emit showStatusMessage(trText("msg_lightmapper_success"));
    } else if (exitCode == -999) {
        emit showStatusMessage(trText("msg_lightmapper_cancelled"));
    } else {
        emit showStatusMessage(QString(trText("msg_lightmapper_failed")).arg(exitCode));
    }
}

void FloatingToolbar::updateProfileButton()
{
    QString activeId = ConfigManager::instance().activeProfileId();
    LightmapProfile p = ConfigManager::instance().profileById(activeId);

    m_profileBtn->setText(QString("💡 %1 ▾").arg(p.displayName));
    m_profileBtn->setToolTip(QString(trText("profile_tooltip"))
        .arg(p.displayName).arg(p.lightmapping).arg(p.lightmaptexsize).arg(p.lightmapquality).arg(p.lightao));
}

void FloatingToolbar::applyAlwaysOnTopFlag(bool onTop)
{
    Qt::WindowFlags flags = windowFlags();
    if (onTop) {
        flags |= Qt::WindowStaysOnTopHint;
    } else {
        flags &= ~Qt::WindowStaysOnTopHint;
    }
    setWindowFlags(flags);
    show();
}

void FloatingToolbar::toggleAlwaysOnTop()
{
    bool newState = !ConfigManager::instance().isAlwaysOnTop();
    ConfigManager::instance().setAlwaysOnTop(newState);
    m_pinBtn->setProperty("pinned", newState);
    m_pinBtn->style()->unpolish(m_pinBtn);
    m_pinBtn->style()->polish(m_pinBtn);
    applyAlwaysOnTopFlag(newState);
}

void FloatingToolbar::updateStatusIndicator()
{
    bool running = ProcessController::instance().isFPSCRunning();

    QPixmap pixmap(48, 48);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    // Favicon image filling full area
    QPixmap iconPix(":/app.png");
    if (iconPix.isNull()) iconPix = QPixmap(":/app.ico");
    if (!iconPix.isNull()) {
        painter.drawPixmap(0, 0, 48, 48, iconPix);
    }

    // Status LED dot overlay (Bottom Right with dark border)
    QColor dot = running ? QColor(0, 230, 118) : QColor(255, 82, 82);
    painter.setBrush(dot);
    painter.setPen(QPen(QColor(18, 20, 24), 2.0));
    painter.drawEllipse(QPoint(38, 38), 5, 5);

    m_statusIconBtn->setIcon(QIcon(pixmap));
    m_statusIconBtn->setToolTip(running ? trText("icon_restart_tooltip_running") : trText("icon_restart_tooltip_stopped"));
}

void FloatingToolbar::switchProfile(const QString &profileId)
{
    ConfigManager &cfg = ConfigManager::instance();
    QString fpscDir = cfg.fpscDirectory();

    if (fpscDir.isEmpty() || !cfg.isValidFpscDirectory(fpscDir)) {
        QMessageBox::warning(this, "FPS Creator", trText("msg_folder_not_set"));
        openSettings();
        return;
    }

    LightmapProfile profile = cfg.profileById(profileId);
    cfg.setActiveProfileId(profileId);
    rebuildProfileMenu();

    // Auto clean if enabled
    if (cfg.autoCleanOnSwitch()) {
        FastCleaner::cleanBinAndDbo(fpscDir);
    }

    // Apply to setup.ini and restart
    SetupIniManager::instance().applyProfile(fpscDir, profile, true);
    updateStatusIndicator();

    emit showStatusMessage(QString(trText("msg_profile_activated")).arg(profile.displayName));
}

void FloatingToolbar::restartFpsc()
{
    ConfigManager &cfg = ConfigManager::instance();
    QString fpscDir = cfg.fpscDirectory();

    if (fpscDir.isEmpty() || !cfg.isValidFpscDirectory(fpscDir)) {
        QMessageBox::warning(this, "FPS Creator", trText("msg_folder_not_set"));
        openSettings();
        return;
    }

    ProcessController::instance().restartFPSC(fpscDir);
    updateStatusIndicator();
    emit showStatusMessage(trText("msg_restarted"));
}

void FloatingToolbar::killFpsc()
{
    int killed = ProcessController::instance().killFPSC();
    updateStatusIndicator();
    emit showStatusMessage(QString(trText("msg_killed")).arg(killed));
}

void FloatingToolbar::cleanCache(CleanType type)
{
    QString fpscDir = ConfigManager::instance().fpscDirectory();
    if (fpscDir.isEmpty() || !ConfigManager::instance().isValidFpscDirectory(fpscDir)) {
        QMessageBox::warning(this, "FPS Creator", trText("msg_folder_not_set"));
        openSettings();
        return;
    }

    CleanResult res = FastCleaner::instance().cleanSync(fpscDir, type);
    emit showStatusMessage(res.summary());
}

void FloatingToolbar::quickStashSave()
{
    QString fpscDir = ConfigManager::instance().fpscDirectory();
    if (fpscDir.isEmpty() || !ConfigManager::instance().isValidFpscDirectory(fpscDir)) {
        QMessageBox::warning(this, "FPS Creator", trText("msg_folder_not_set"));
        openSettings();
        return;
    }

    QString error;
    if (StashManager::instance().quickSaveStash(fpscDir, &error)) {
        emit showStatusMessage(trText("msg_stash_saved"));
    } else {
        QMessageBox::critical(this, "Error", QString("Failed to save stash:\n%1").arg(error));
    }
}

void FloatingToolbar::quickStashRestore()
{
    QString fpscDir = ConfigManager::instance().fpscDirectory();
    if (fpscDir.isEmpty() || !ConfigManager::instance().isValidFpscDirectory(fpscDir)) {
        QMessageBox::warning(this, "FPS Creator", trText("msg_folder_not_set"));
        openSettings();
        return;
    }

    int res = QMessageBox::warning(this, "Restore testlevel",
        trText("stash_restore_confirm"),
        QMessageBox::Yes | QMessageBox::No);

    if (res == QMessageBox::Yes) {
        QString error;
        if (StashManager::instance().quickRestoreStash(fpscDir, &error)) {
            emit showStatusMessage(trText("msg_stash_restored"));
        } else {
            QMessageBox::critical(this, "Error", QString("Failed to restore stash:\n%1").arg(error));
        }
    }
}

void FloatingToolbar::openStashManager()
{
    StashDialog dlg(this);
    dlg.exec();
}

void FloatingToolbar::openStashFolder()
{
    QString fpscDir = ConfigManager::instance().fpscDirectory();
    QString stashesRoot = StashManager::instance().stashesRootDirectory(fpscDir);
    if (!stashesRoot.isEmpty() && QDir(stashesRoot).exists()) {
        QDesktopServices::openUrl(QUrl::fromLocalFile(stashesRoot));
    } else {
        QMessageBox::information(this, "Stash", trText("stash_no_folder_yet"));
    }
}

void FloatingToolbar::openSettings()
{
    SettingsDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        applyAlwaysOnTopFlag(ConfigManager::instance().isAlwaysOnTop());
        m_pinBtn->setProperty("pinned", ConfigManager::instance().isAlwaysOnTop());
        m_pinBtn->style()->unpolish(m_pinBtn);
        m_pinBtn->style()->polish(m_pinBtn);

        // Recreate profile dropdown if profiles changed
        rebuildProfileMenu();
        retranslateUi();
    }
}

void FloatingToolbar::foldToMiniIcon()
{
    ConfigManager::instance().setToolbarPosition(pos());
    emit foldRequested();
}

void FloatingToolbar::minimizeToTray()
{
    ConfigManager::instance().setToolbarPosition(pos());
    emit minimizeToTrayRequested();
}

void FloatingToolbar::checkHoverState()
{
    if (m_isDragging) return;

    // If any context or dropdown menu is currently active, keep topBar visible
    if ((m_profileMenu && m_profileMenu->isVisible()) ||
        (m_cleanMenu && m_cleanMenu->isVisible()) ||
        (m_stashMenu && m_stashMenu->isVisible())) {
        return;
    }

    QPoint cursorPos = QCursor::pos();
    QRect activeZone = frameGeometry().adjusted(-4, -4, 4, 4);

    bool shouldExpand = activeZone.contains(cursorPos);

    if (shouldExpand != m_topBarWidget->isVisible()) {
        m_topBarWidget->setVisible(shouldExpand);
    }
}

void FloatingToolbar::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragPosition = event->globalPos() - frameGeometry().topLeft();
        m_isDragging = true;
        event->accept();
    }
}

void FloatingToolbar::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton && m_isDragging) {
        move(event->globalPos() - m_dragPosition);
        ConfigManager::instance().setToolbarPosition(pos());
        event->accept();
    }
}

void FloatingToolbar::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_isDragging = false;
        event->accept();
    }
}

void FloatingToolbar::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu(this);

    QAction *restartAct = menu.addAction(trText("menu_restart"));
    connect(restartAct, &QAction::triggered, this, &FloatingToolbar::restartFpsc);

    QAction *killAct = menu.addAction(trText("menu_kill"));
    connect(killAct, &QAction::triggered, this, &FloatingToolbar::killFpsc);

    menu.addSeparator();

    QAction *foldAct = menu.addAction(trText("menu_fold"));
    connect(foldAct, &QAction::triggered, this, &FloatingToolbar::foldToMiniIcon);

    QAction *trayAct = menu.addAction(trText("menu_tray"));
    connect(trayAct, &QAction::triggered, this, &FloatingToolbar::minimizeToTray);

    menu.addSeparator();

    QAction *settingsAct = menu.addAction(trText("menu_settings"));
    connect(settingsAct, &QAction::triggered, this, &FloatingToolbar::openSettings);

    QAction *exitAct = menu.addAction(trText("menu_exit"));
    connect(exitAct, &QAction::triggered, qApp, &QApplication::quit);

    menu.exec(event->globalPos());
}
