#include "StashDialog.h"
#include "ConfigManager.h"
#include "Theme.h"
#include "I18n.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QMessageBox>
#include <QDesktopServices>
#include <QUrl>
#include <QDir>

StashDialog::StashDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(QString("%1 — v1.0.0 beta").arg(trText("stash_title")));
    setStyleSheet(Theme::darkStyleSheet());
    resize(640, 420);
    setModal(true);

    setupUi();
    refreshList();
}

void StashDialog::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);

    QLabel *infoLabel = new QLabel(trText("stash_info_label"), this);
    mainLayout->addWidget(infoLabel);

    m_table = new QTableWidget(this);
    m_table->setColumnCount(4);
    m_table->setHorizontalHeaderLabels({trText("stash_col_name"), trText("stash_col_date"), trText("stash_col_files"), trText("stash_col_size")});
    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->verticalHeader()->setVisible(false);
    mainLayout->addWidget(m_table, 1);

    m_statusLabel = new QLabel(this);
    mainLayout->addWidget(m_statusLabel);

    // Action Buttons
    QHBoxLayout *btnRow = new QHBoxLayout();

    m_createBtn = new QPushButton(trText("stash_btn_create"), this);
    connect(m_createBtn, &QPushButton::clicked, this, &StashDialog::onCreateStash);

    m_restoreBtn = new QPushButton(trText("stash_btn_restore"), this);
    connect(m_restoreBtn, &QPushButton::clicked, this, &StashDialog::onRestoreSelected);

    m_deleteBtn = new QPushButton(trText("stash_btn_delete"), this);
    connect(m_deleteBtn, &QPushButton::clicked, this, &StashDialog::onDeleteSelected);

    m_openFolderBtn = new QPushButton(trText("stash_btn_open"), this);
    connect(m_openFolderBtn, &QPushButton::clicked, this, &StashDialog::onOpenInExplorer);

    m_closeBtn = new QPushButton(trText("stash_btn_close"), this);
    connect(m_closeBtn, &QPushButton::clicked, this, &QDialog::accept);

    btnRow->addWidget(m_createBtn);
    btnRow->addWidget(m_restoreBtn);
    btnRow->addWidget(m_deleteBtn);
    btnRow->addWidget(m_openFolderBtn);
    btnRow->addStretch();
    btnRow->addWidget(m_closeBtn);

    mainLayout->addLayout(btnRow);
}

void StashDialog::refreshList()
{
    QString fpscDir = ConfigManager::instance().fpscDirectory();
    m_stashes = StashManager::instance().listStashes(fpscDir);

    m_table->setRowCount(m_stashes.size());

    for (int i = 0; i < m_stashes.size(); ++i) {
        const StashInfo &info = m_stashes[i];

        QTableWidgetItem *nameItem = new QTableWidgetItem(info.name);
        nameItem->setData(Qt::UserRole, info.id);
        if (!info.fpmRelativePath.isEmpty()) {
            nameItem->setToolTip(QString("🗺 %1: %2").arg(trText("stash_map_file")).arg(info.fpmRelativePath));
        } else if (!info.fpmSourcePath.isEmpty()) {
            nameItem->setToolTip(QString("🗺 %1: %2").arg(trText("stash_map_file")).arg(QFileInfo(info.fpmSourcePath).fileName()));
        }

        QTableWidgetItem *dateItem = new QTableWidgetItem(info.formattedTime());
        QTableWidgetItem *countItem = new QTableWidgetItem(QString::number(info.fileCount));
        QTableWidgetItem *sizeItem = new QTableWidgetItem(info.formattedBytes());

        m_table->setItem(i, 0, nameItem);
        m_table->setItem(i, 1, dateItem);
        m_table->setItem(i, 2, countItem);
        m_table->setItem(i, 3, sizeItem);
    }

    if (m_stashes.isEmpty()) {
        m_statusLabel->setText(QString("<span style='color:#7a8292;'>%1</span>").arg(trText("stash_empty_label")));
        m_restoreBtn->setEnabled(false);
        m_deleteBtn->setEnabled(false);
        m_openFolderBtn->setEnabled(false);
    } else {
        m_statusLabel->setText(QString(trText("stash_total_count")).arg(m_stashes.size()));
        m_restoreBtn->setEnabled(true);
        m_deleteBtn->setEnabled(true);
        m_openFolderBtn->setEnabled(true);
        if (m_table->currentRow() < 0) {
            m_table->selectRow(0);
        }
    }
}

QString StashDialog::currentSelectedStashId() const
{
    int row = m_table->currentRow();
    if (row >= 0 && row < m_stashes.size()) {
        return m_stashes[row].id;
    }
    return "";
}

void StashDialog::onCreateStash()
{
    QString fpscDir = ConfigManager::instance().fpscDirectory();
    if (fpscDir.isEmpty() || !ConfigManager::instance().isValidFpscDirectory(fpscDir)) {
        QMessageBox::warning(this, "FPS Creator", trText("msg_folder_not_set"));
        return;
    }

    bool ok = false;
    QString defaultPrefix = I18n::instance().isRussian() ? "Снимок" : "Snapshot";
    QString defaultName = QString("%1 %2").arg(defaultPrefix).arg(QDateTime::currentDateTime().toString("dd.MM.yyyy HH:mm:ss"));
    QString name = QInputDialog::getText(this, trText("stash_create_title"), trText("stash_create_prompt"), QLineEdit::Normal, defaultName, &ok);

    if (ok && !name.trimmed().isEmpty()) {
        StashInfo info;
        QString error;
        if (StashManager::instance().createStash(fpscDir, name.trimmed(), &info, &error)) {
            refreshList();
            m_statusLabel->setText(QString("<span style='color:#66bb6a;'>✔ %1 (%2 files, %3)</span>")
                .arg(info.name).arg(info.fileCount).arg(info.formattedBytes()));
        } else {
            QMessageBox::critical(this, "Error", QString("Failed to create snapshot:\n%1").arg(error));
        }
    }
}

void StashDialog::onRestoreSelected()
{
    QString stashId = currentSelectedStashId();
    if (stashId.isEmpty()) return;

    QString fpscDir = ConfigManager::instance().fpscDirectory();
    int res = QMessageBox::warning(this, "Restore",
        trText("stash_restore_confirm"),
        QMessageBox::Yes | QMessageBox::No);

    if (res == QMessageBox::Yes) {
        QString error;
        if (StashManager::instance().restoreStash(fpscDir, stashId, &error)) {
            m_statusLabel->setText(QString("<span style='color:#66bb6a;'>✔ %1</span>").arg(trText("msg_stash_restored")));
            QMessageBox::information(this, "Restore", trText("msg_stash_restored"));
        } else {
            QMessageBox::critical(this, "Error", QString("Failed to restore snapshot:\n%1").arg(error));
        }
    }
}

void StashDialog::onDeleteSelected()
{
    QString stashId = currentSelectedStashId();
    if (stashId.isEmpty()) return;

    int res = QMessageBox::question(this, "Delete", trText("stash_delete_confirm"), QMessageBox::Yes | QMessageBox::No);
    if (res == QMessageBox::Yes) {
        QString fpscDir = ConfigManager::instance().fpscDirectory();
        QString error;
        if (StashManager::instance().deleteStash(fpscDir, stashId, &error)) {
            refreshList();
            m_statusLabel->setText(QString("<span style='color:#ffa726;'>%1</span>").arg(trText("stash_delete_success")));
        } else {
            QMessageBox::critical(this, "Error", QString("Failed to delete snapshot:\n%1").arg(error));
        }
    }
}

void StashDialog::onOpenInExplorer()
{
    QString fpscDir = ConfigManager::instance().fpscDirectory();
    QString stashesRoot = StashManager::instance().stashesRootDirectory(fpscDir);
    if (QDir(stashesRoot).exists()) {
        QDesktopServices::openUrl(QUrl::fromLocalFile(stashesRoot));
    }
}
