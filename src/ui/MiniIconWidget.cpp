#include "MiniIconWidget.h"
#include "ConfigManager.h"
#include "I18n.h"
#include <QPainter>
#include <QMouseEvent>
#include <QApplication>
#include <QPainterPath>
#include <QAction>
#include <QActionGroup>

MiniIconWidget::MiniIconWidget(QWidget *parent)
    : QWidget(parent, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool)
    , m_isDragging(false)
    , m_isFpscRunning(false)
    , m_activeProfileName("Normal")
    , m_contextMenu(nullptr)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(52, 52);
    setToolTip("FPSC Tool");
    setCursor(Qt::PointingHandCursor);

    createContextMenu();
}

void MiniIconWidget::updateStatus(bool isFpscRunning, const QString &activeProfile)
{
    m_isFpscRunning = isFpscRunning;
    m_activeProfileName = activeProfile;
    update();
}

void MiniIconWidget::createContextMenu()
{
    if (m_contextMenu) {
        delete m_contextMenu;
    }

    m_contextMenu = new QMenu(this);

    QAction *expandAct = m_contextMenu->addAction(trText("menu_expand"));
    connect(expandAct, &QAction::triggered, this, &MiniIconWidget::expandRequested);

    m_contextMenu->addSeparator();

    // Profiles Submenu
    QMenu *profilesMenu = m_contextMenu->addMenu(trText("menu_profiles"));
    QActionGroup *profileGroup = new QActionGroup(profilesMenu);
    
    QVector<LightmapProfile> profiles = ConfigManager::instance().profiles();
    QString activeId = ConfigManager::instance().activeProfileId();

    for (const LightmapProfile &p : profiles) {
        QAction *act = profilesMenu->addAction(p.displayName);
        act->setCheckable(true);
        act->setChecked(p.id == activeId);
        profileGroup->addAction(act);
        connect(act, &QAction::triggered, this, [this, p]() {
            emit profileSelected(p.id);
        });
    }

    // NOTE: Manual LightMapper execution commented out - functionality does not work completely
    /*
    profilesMenu->addSeparator();
    QAction *runLmAct = profilesMenu->addAction(trText("menu_run_lightmapper"));
    connect(runLmAct, &QAction::triggered, this, &MiniIconWidget::runLightMapperRequested);
    */

    m_contextMenu->addSeparator();

    QAction *restartAct = m_contextMenu->addAction(trText("menu_restart"));
    connect(restartAct, &QAction::triggered, this, &MiniIconWidget::restartRequested);

    QAction *killAct = m_contextMenu->addAction(trText("menu_kill"));
    connect(killAct, &QAction::triggered, this, &MiniIconWidget::killRequested);

    m_contextMenu->addSeparator();

    // Clean Submenu
    QMenu *cleanMenu = m_contextMenu->addMenu(trText("menu_clean"));
    QAction *cleanBinAct = cleanMenu->addAction(trText("clean_bin_dbo"));
    connect(cleanBinAct, &QAction::triggered, this, [this]() { emit cleanRequested(0); });
    
    QAction *cleanLevelAct = cleanMenu->addAction(trText("clean_level_data"));
    connect(cleanLevelAct, &QAction::triggered, this, [this]() { emit cleanRequested(3); });

    QAction *cleanAllAct = cleanMenu->addAction(trText("clean_all"));
    connect(cleanAllAct, &QAction::triggered, this, [this]() { emit cleanRequested(2); });

    cleanMenu->addSeparator();

    QAction *cleanTempAct = cleanMenu->addAction(trText("clean_engine_temp"));
    connect(cleanTempAct, &QAction::triggered, this, [this]() { emit cleanRequested(4); });

    // Stash Submenu
    QMenu *stashMenu = m_contextMenu->addMenu(trText("menu_stash"));
    QAction *saveStashAct = stashMenu->addAction(trText("stash_save_quick"));
    connect(saveStashAct, &QAction::triggered, this, &MiniIconWidget::stashSaveRequested);

    QAction *restoreStashAct = stashMenu->addAction(trText("stash_restore_quick"));
    connect(restoreStashAct, &QAction::triggered, this, &MiniIconWidget::stashRestoreRequested);

    m_contextMenu->addSeparator();

    QAction *settingsAct = m_contextMenu->addAction(trText("menu_settings"));
    connect(settingsAct, &QAction::triggered, this, &MiniIconWidget::settingsRequested);

    m_contextMenu->addSeparator();

    QAction *exitAct = m_contextMenu->addAction(trText("menu_exit"));
    connect(exitAct, &QAction::triggered, this, &MiniIconWidget::exitRequested);
}

void MiniIconWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    QRect rect = this->rect().adjusted(2, 2, -2, -2);

    // Background rounded rectangle
    QPainterPath path;
    path.addRoundedRect(rect, 12, 12);

    // Outer glow / border
    QColor borderColor = m_isFpscRunning ? QColor(0, 230, 118) : QColor(0, 188, 212);
    painter.fillPath(path, QColor(26, 28, 35, 245));

    QPen pen(borderColor, 2);
    painter.setPen(pen);
    painter.drawPath(path);

    // Favicon Icon (32x32 centered)
    QPixmap iconPix(":/app.png");
    if (iconPix.isNull()) iconPix = QPixmap(":/app.ico");
    if (!iconPix.isNull()) {
        painter.drawPixmap((width() - 32) / 2, (height() - 32) / 2, 32, 32, iconPix);
    }

    // Status Indicator Dot (Top Right Badge)
    QColor dotColor = m_isFpscRunning ? QColor(0, 230, 118) : QColor(255, 82, 82);
    painter.setBrush(dotColor);
    painter.setPen(QPen(QColor(26, 28, 35), 1.5));
    painter.drawEllipse(QPoint(width() - 11, 11), 4, 4);
}

void MiniIconWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragPosition = event->globalPos() - frameGeometry().topLeft();
        m_isDragging = false;
        event->accept();
    }
}

void MiniIconWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        m_isDragging = true;
        move(event->globalPos() - m_dragPosition);
        ConfigManager::instance().setMiniWidgetPosition(pos());
        event->accept();
    }
}

void MiniIconWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        if (!m_isDragging) {
            emit expandRequested();
        }
        m_isDragging = false;
        event->accept();
    }
}

void MiniIconWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        emit expandRequested();
        event->accept();
    }
}

void MiniIconWidget::contextMenuEvent(QContextMenuEvent *event)
{
    createContextMenu();
    m_contextMenu->popup(event->globalPos());
}
