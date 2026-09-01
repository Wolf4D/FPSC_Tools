#ifndef FLOATINGTOOLBAR_H
#define FLOATINGTOOLBAR_H

#include <QWidget>
#include <QPoint>
#include <QPushButton>
#include <QLabel>
#include <QVector>
#include <QMenu>
#include <QTimer>
#include "ConfigManager.h"
#include "FastCleaner.h"

class FloatingToolbar : public QWidget
{
    Q_OBJECT

public:
    explicit FloatingToolbar(QWidget *parent = nullptr);
    ~FloatingToolbar() override = default;

    void updateProfileButton();
    void updateStatusIndicator();
    void retranslateUi();

public slots:
    void switchProfile(const QString &profileId);
    void restartFpsc();
    void killFpsc();
    void cleanCache(CleanType type);
    void quickStashSave();
    void quickStashRestore();
    void openStashManager();
    void openStashFolder();
    void openSettings();
    void toggleAlwaysOnTop();
    void foldToMiniIcon();
    void minimizeToTray();
    void runLightMapper();

signals:
    void foldRequested();
    void minimizeToTrayRequested();
    void showStatusMessage(const QString &msg);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;

private:
    void setupUi();
    void applyAlwaysOnTopFlag(bool onTop);
    void rebuildProfileMenu();
    void rebuildCleanMenu();
    void rebuildStashMenu();
    void checkHoverState();

    QPoint m_dragPosition;
    bool m_isDragging;

    QWidget *m_topBarWidget;
    QWidget *m_contentWidget;
    QLabel *m_titleGripLabel;
    QPushButton *m_statusIconBtn;

    QPushButton *m_profileBtn;
    QMenu *m_profileMenu;

    QPushButton *m_restartBtn;

    QPushButton *m_cleanBtn;
    QMenu *m_cleanMenu;

    QPushButton *m_stashBtn;
    QMenu *m_stashMenu;

    QPushButton *m_pinBtn;
    QPushButton *m_foldBtn;
    QPushButton *m_settingsBtn;
    QPushButton *m_closeBtn;

    QTimer *m_statusTimer;
    QTimer *m_hoverTracker;
};

#endif // FLOATINGTOOLBAR_H
