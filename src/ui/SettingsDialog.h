#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QComboBox>
#include <QCheckBox>
#include <QLabel>
#include "ConfigManager.h"

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog() override = default;

private slots:
    void onBrowseDirectory();
    void onAutoDetectDirectory();
    void onResetProfiles();
    void onSave();

private:
    void setupUi();
    void loadFromConfig();
    void updatePathValidation();

    QLineEdit *m_pathEdit;
    QLabel *m_pathStatusLabel;
    QPushButton *m_browseBtn;
    QPushButton *m_autoDetectBtn;

    QTableWidget *m_profilesTable;
    QPushButton *m_resetProfilesBtn;

    QComboBox *m_languageCombo;
    QCheckBox *m_autoCleanCheck;
    QCheckBox *m_alwaysOnTopCheck;
    QCheckBox *m_showNotificationsCheck;
    QCheckBox *m_startMinimizedCheck;

    QPushButton *m_saveBtn;
    QPushButton *m_cancelBtn;

    QVector<LightmapProfile> m_workingProfiles;
};

#endif // SETTINGSDIALOG_H
