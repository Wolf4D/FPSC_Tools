#ifndef STASHDIALOG_H
#define STASHDIALOG_H

#include <QDialog>
#include <QTableWidget>
#include <QPushButton>
#include <QLabel>
#include "StashManager.h"

class StashDialog : public QDialog
{
    Q_OBJECT

public:
    explicit StashDialog(QWidget *parent = nullptr);
    ~StashDialog() override = default;

public slots:
    void refreshList();

private slots:
    void onCreateStash();
    void onRestoreSelected();
    void onDeleteSelected();
    void onOpenInExplorer();

private:
    void setupUi();
    QString currentSelectedStashId() const;

    QTableWidget *m_table;
    QPushButton *m_createBtn;
    QPushButton *m_restoreBtn;
    QPushButton *m_deleteBtn;
    QPushButton *m_openFolderBtn;
    QPushButton *m_closeBtn;
    QLabel *m_statusLabel;

    QVector<StashInfo> m_stashes;
};

#endif // STASHDIALOG_H
