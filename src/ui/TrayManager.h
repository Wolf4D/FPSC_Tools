#ifndef TRAYMANAGER_H
#define TRAYMANAGER_H

#include <QObject>
#include <QSystemTrayIcon>
#include <QMenu>

class TrayManager : public QObject
{
    Q_OBJECT

public:
    explicit TrayManager(QObject *parent = nullptr);
    ~TrayManager() override = default;

    void initTray();
    void showNotification(const QString &title, const QString &message, QSystemTrayIcon::MessageIcon icon = QSystemTrayIcon::Information);
    void updateMenu();

signals:
    void toggleToolbarRequested();
    void showToolbarRequested();
    void profileSelected(const QString &profileId);
    void runLightMapperRequested();
    void restartRequested();
    void killRequested();
    void cleanRequested(int type);
    void stashSaveRequested();
    void stashRestoreRequested();
    void stashManagerRequested();
    void settingsRequested();
    void exitRequested();

private slots:
    void onTrayActivated(QSystemTrayIcon::ActivationReason reason);

private:
    void createMenu();
    QIcon createTrayIcon(bool isFpscRunning = false);

    QSystemTrayIcon *m_trayIcon;
    QMenu *m_trayMenu;
};

#endif // TRAYMANAGER_H
