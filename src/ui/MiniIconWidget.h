#ifndef MINIICONWIDGET_H
#define MINIICONWIDGET_H

#include <QWidget>
#include <QPoint>
#include <QMenu>

class MiniIconWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MiniIconWidget(QWidget *parent = nullptr);
    ~MiniIconWidget() override = default;

    void updateStatus(bool isFpscRunning, const QString &activeProfile);

signals:
    void expandRequested();
    void runLightMapperRequested();
    void restartRequested();
    void killRequested();
    void cleanRequested(int cleanType);
    void stashSaveRequested();
    void stashRestoreRequested();
    void settingsRequested();
    void exitRequested();
    void profileSelected(const QString &profileId);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;

private:
    void createContextMenu();

    QPoint m_dragPosition;
    bool m_isDragging;
    bool m_isFpscRunning;
    QString m_activeProfileName;
    QMenu *m_contextMenu;
};

#endif // MINIICONWIDGET_H
