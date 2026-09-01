#ifndef APPCONTROLLER_H
#define APPCONTROLLER_H

#include <QObject>
#include "FloatingToolbar.h"
#include "MiniIconWidget.h"
#include "TrayManager.h"

class AppController : public QObject
{
    Q_OBJECT

public:
    explicit AppController(QObject *parent = nullptr);
    ~AppController() override;

    void start();

public slots:
    void showFloatingToolbar();
    void showMiniIconWidget();
    void toggleToolbarVisibility();
    void hideAllToTray();

    void onProfileSelected(const QString &profileId);
    void onRestartRequested();
    void onKillRequested();
    void onCleanRequested(int type);
    void onStashSaveRequested();
    void onStashRestoreRequested();
    void onStashManagerRequested();
    void onSettingsRequested();
    void onExitRequested();
    void onStatusMessage(const QString &msg);

private:
    void setupConnections();

    FloatingToolbar *m_toolbar;
    MiniIconWidget *m_miniWidget;
    TrayManager *m_trayManager;
    QPoint m_foldOffsetFromToolbar;
};

#endif // APPCONTROLLER_H
