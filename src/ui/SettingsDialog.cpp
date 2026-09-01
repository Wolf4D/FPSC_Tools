#include "SettingsDialog.h"
#include "I18n.h"
#include "Theme.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QFileDialog>
#include <QHeaderView>
#include <QComboBox>
#include <QSpinBox>
#include <QMessageBox>
#include <QScrollBar>
#include <QEvent>

namespace {
class NoWheelFilter : public QObject {
public:
    explicit NoWheelFilter(QObject *parent = nullptr) : QObject(parent) {}
protected:
    bool eventFilter(QObject *watched, QEvent *event) override {
        if (event->type() == QEvent::Wheel) {
            event->accept();
            return true;
        }
        return QObject::eventFilter(watched, event);
    }
};
}

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(QString("%1 — v1.0.0 beta").arg(trText("settings_title")));
    setStyleSheet(Theme::darkStyleSheet());
    resize(640, 580);
    setModal(true);

    setupUi();
    loadFromConfig();
}

void SettingsDialog::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(14, 14, 14, 14);

    // 1. Group: FPS Creator Directory
    QGroupBox *dirGroup = new QGroupBox(trText("settings_dir_group"), this);
    QVBoxLayout *dirLayout = new QVBoxLayout(dirGroup);

    QHBoxLayout *pathRow = new QHBoxLayout();
    m_pathEdit = new QLineEdit(this);
    m_pathEdit->setPlaceholderText(trText("settings_dir_placeholder"));
    connect(m_pathEdit, &QLineEdit::textChanged, this, &SettingsDialog::updatePathValidation);

    m_browseBtn = new QPushButton(trText("settings_browse"), this);
    connect(m_browseBtn, &QPushButton::clicked, this, &SettingsDialog::onBrowseDirectory);

    m_autoDetectBtn = new QPushButton(trText("settings_autodetect"), this);
    connect(m_autoDetectBtn, &QPushButton::clicked, this, &SettingsDialog::onAutoDetectDirectory);

    pathRow->addWidget(m_pathEdit, 1);
    pathRow->addWidget(m_browseBtn);
    pathRow->addWidget(m_autoDetectBtn);
    dirLayout->addLayout(pathRow);

    m_pathStatusLabel = new QLabel(this);
    dirLayout->addWidget(m_pathStatusLabel);

    mainLayout->addWidget(dirGroup);

    // 2. Group: Lightmapping Profiles
    QGroupBox *profilesGroup = new QGroupBox(trText("settings_profiles_group"), this);
    QVBoxLayout *profLayout = new QVBoxLayout(profilesGroup);

    m_profilesTable = new QTableWidget(this);
    m_profilesTable->setColumnCount(5);
    QString profileHeader = I18n::instance().isRussian() ? "Профиль" : "Profile";
    m_profilesTable->setHorizontalHeaderLabels({profileHeader, "lightmapping", "lightmaptexsize", "lightmapquality", "lightao"});
    m_profilesTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_profilesTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_profilesTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_profilesTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    m_profilesTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    m_profilesTable->verticalHeader()->setVisible(false);
    m_profilesTable->verticalHeader()->setDefaultSectionSize(30);
    m_profilesTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_profilesTable->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_profilesTable->installEventFilter(new NoWheelFilter(m_profilesTable));
    m_profilesTable->viewport()->installEventFilter(new NoWheelFilter(m_profilesTable->viewport()));
    m_profilesTable->verticalScrollBar()->setEnabled(false);
    profLayout->addWidget(m_profilesTable);

    QHBoxLayout *profBtnsRow = new QHBoxLayout();
    m_resetProfilesBtn = new QPushButton(trText("settings_reset_profiles"), this);
    connect(m_resetProfilesBtn, &QPushButton::clicked, this, &SettingsDialog::onResetProfiles);
    profBtnsRow->addStretch();
    profBtnsRow->addWidget(m_resetProfilesBtn);
    profLayout->addLayout(profBtnsRow);

    mainLayout->addWidget(profilesGroup, 1);

    // 3. Group: General Settings
    QGroupBox *genGroup = new QGroupBox(trText("settings_general_group"), this);
    QVBoxLayout *genLayout = new QVBoxLayout(genGroup);

    QHBoxLayout *langRow = new QHBoxLayout();
    QLabel *langLabel = new QLabel(trText("settings_language"), this);
    m_languageCombo = new QComboBox(this);
    m_languageCombo->installEventFilter(new NoWheelFilter(m_languageCombo));
    m_languageCombo->addItem("Auto (System)", "auto");
    m_languageCombo->addItem("English", "en");
    m_languageCombo->addItem("Русский (Russian)", "ru");
    langRow->addWidget(langLabel);
    langRow->addWidget(m_languageCombo);
    langRow->addStretch();
    genLayout->addLayout(langRow);

    m_autoCleanCheck = new QCheckBox(trText("settings_auto_clean"), this);
    m_alwaysOnTopCheck = new QCheckBox(trText("settings_always_on_top"), this);
    m_showNotificationsCheck = new QCheckBox(trText("settings_show_notifications"), this);
    m_startMinimizedCheck = new QCheckBox(trText("settings_start_minimized"), this);

    genLayout->addWidget(m_autoCleanCheck);
    genLayout->addWidget(m_alwaysOnTopCheck);
    genLayout->addWidget(m_showNotificationsCheck);
    genLayout->addWidget(m_startMinimizedCheck);

    mainLayout->addWidget(genGroup);

    // 4. Bottom Dialog Buttons & Credits (Save then Cancel)
    QHBoxLayout *bottomRow = new QHBoxLayout();

    QLabel *creditsLabel = new QLabel(this);
    creditsLabel->setText(QString("<span style='font-size:11px; color:#8a99ad;'><b>FPSC Tools</b> 1.0.0 beta &bull; <b>Madness Studio</b> &bull; Ivan Klenov (aka Navy LiK)<br><span style='color:#00bcd4; font-weight:bold;'>%1</span></span>")
                          .arg(trText("app_bima_support")));
    creditsLabel->setStyleSheet("background: transparent; border: none; padding: 2px 0px;");
    bottomRow->addWidget(creditsLabel, 1);

    m_saveBtn = new QPushButton(trText("settings_save"), this);
    m_saveBtn->setDefault(true);
    connect(m_saveBtn, &QPushButton::clicked, this, &SettingsDialog::onSave);

    m_cancelBtn = new QPushButton(trText("settings_cancel"), this);
    connect(m_cancelBtn, &QPushButton::clicked, this, &QDialog::reject);

    bottomRow->addWidget(m_saveBtn);
    bottomRow->addWidget(m_cancelBtn);
    mainLayout->addLayout(bottomRow);
}

void SettingsDialog::loadFromConfig()
{
    ConfigManager &cfg = ConfigManager::instance();
    m_pathEdit->setText(cfg.fpscDirectory());
    updatePathValidation();

    QString currentLang = cfg.language();
    int langIdx = m_languageCombo->findData(currentLang);
    if (langIdx != -1) m_languageCombo->setCurrentIndex(langIdx);
    else m_languageCombo->setCurrentIndex(0);

    m_autoCleanCheck->setChecked(cfg.autoCleanOnSwitch());
    m_alwaysOnTopCheck->setChecked(cfg.isAlwaysOnTop());
    m_showNotificationsCheck->setChecked(cfg.showNotifications());
    m_startMinimizedCheck->setChecked(cfg.startMinimized());

    m_workingProfiles = cfg.profiles();

    m_profilesTable->setRowCount(m_workingProfiles.size());
    for (int r = 0; r < m_workingProfiles.size(); ++r) {
        const LightmapProfile &p = m_workingProfiles[r];

        // Column 0: Display Name
        QTableWidgetItem *nameItem = new QTableWidgetItem(p.displayName);
        nameItem->setFlags(nameItem->flags() & ~Qt::ItemIsEditable);
        m_profilesTable->setItem(r, 0, nameItem);

        // Column 1: lightmapping (0 / 1)
        QComboBox *lmCombo = new QComboBox(m_profilesTable);
        lmCombo->installEventFilter(new NoWheelFilter(lmCombo));
        lmCombo->addItem(I18n::instance().isRussian() ? "0 (Выкл)" : "0 (Off)", 0);
        lmCombo->addItem(I18n::instance().isRussian() ? "1 (Вкл)" : "1 (On)", 1);
        lmCombo->setCurrentIndex(p.lightmapping == 1 ? 1 : 0);
        m_profilesTable->setCellWidget(r, 1, lmCombo);

        // Column 2: lightmaptexsize
        QComboBox *texCombo = new QComboBox(m_profilesTable);
        texCombo->installEventFilter(new NoWheelFilter(texCombo));
        QList<int> sizes = {16, 32, 64, 128, 256, 512, 1024, 2048};
        for (int s : sizes) {
            texCombo->addItem(QString::number(s), s);
        }
        int idx = sizes.indexOf(p.lightmaptexsize);
        if (idx != -1) texCombo->setCurrentIndex(idx);
        else {
            texCombo->addItem(QString::number(p.lightmaptexsize), p.lightmaptexsize);
            texCombo->setCurrentIndex(texCombo->count() - 1);
        }
        m_profilesTable->setCellWidget(r, 2, texCombo);

        // Column 3: lightmapquality
        QSpinBox *qualSpin = new QSpinBox(m_profilesTable);
        qualSpin->installEventFilter(new NoWheelFilter(qualSpin));
        qualSpin->setRange(1, 300);
        qualSpin->setValue(p.lightmapquality);
        m_profilesTable->setCellWidget(r, 3, qualSpin);

        // Column 4: lightao (0 / 1)
        QComboBox *aoCombo = new QComboBox(m_profilesTable);
        aoCombo->installEventFilter(new NoWheelFilter(aoCombo));
        aoCombo->addItem(I18n::instance().isRussian() ? "0 (Выкл)" : "0 (Off)", 0);
        aoCombo->addItem(I18n::instance().isRussian() ? "1 (Вкл)" : "1 (On)", 1);
        aoCombo->setCurrentIndex(p.lightao == 1 ? 1 : 0);
        m_profilesTable->setRowHeight(r, 28);
        m_profilesTable->setCellWidget(r, 4, aoCombo);
    }

    int headerH = m_profilesTable->horizontalHeader()->sizeHint().height();
    if (headerH < 28) headerH = 28;
    int tableH = headerH + (28 * m_workingProfiles.size()) + 4;
    m_profilesTable->setFixedHeight(tableH);
    m_profilesTable->verticalScrollBar()->setValue(0);
}

void SettingsDialog::updatePathValidation()
{
    QString path = m_pathEdit->text().trimmed();
    bool valid = ConfigManager::instance().isValidFpscDirectory(path);

    if (path.isEmpty()) {
        m_pathStatusLabel->setText(QString("<span style='color:#ffa726;'>%1</span>").arg(trText("settings_empty_path")));
    } else if (valid) {
        m_pathStatusLabel->setText(QString("<span style='color:#66bb6a;'>%1</span>").arg(trText("settings_valid_path")));
    } else {
        m_pathStatusLabel->setText(QString("<span style='color:#ef5350;'>%1</span>").arg(trText("settings_invalid_path")));
    }
}

void SettingsDialog::onBrowseDirectory()
{
    QString initial = m_pathEdit->text();
    if (initial.isEmpty() || !QDir(initial).exists()) {
        initial = "C:/";
    }

    QString title = I18n::instance().isRussian() ? "Выберите корневую папку FPS Creator" : "Select FPS Creator Root Folder";
    QString dir = QFileDialog::getExistingDirectory(this, title, initial);
    if (!dir.isEmpty()) {
        m_pathEdit->setText(dir);
    }
}

void SettingsDialog::onAutoDetectDirectory()
{
    QString found = ConfigManager::instance().autoDetectFpscDirectory();
    if (!found.isEmpty()) {
        m_pathEdit->setText(found);
    } else {
        QMessageBox::information(this, trText("settings_autodetect"), trText("settings_autodetect_fail"));
    }
}

void SettingsDialog::onResetProfiles()
{
    int res = QMessageBox::question(this, trText("settings_reset_profiles"), trText("settings_reset_confirm"), QMessageBox::Yes | QMessageBox::No);
    if (res == QMessageBox::Yes) {
        // Reset to user-approved standard presets
        for (LightmapProfile &p : m_workingProfiles) {
            if (p.id == "disabled") {
                p.displayName = trText("profile_disabled");
                p.lightmapping = 0;
                p.lightmaptexsize = 16;
                p.lightmapquality = 16;
                p.lightao = 0;
            } else if (p.id == "fast") {
                p.displayName = trText("profile_fast");
                p.lightmapping = 1;
                p.lightmaptexsize = 128;
                p.lightmapquality = 5;
                p.lightao = 0;
            } else if (p.id == "normal") {
                p.displayName = trText("profile_normal");
                p.lightmapping = 1;
                p.lightmaptexsize = 256;
                p.lightmapquality = 16;
                p.lightao = 0;
            } else if (p.id == "release") {
                p.displayName = trText("profile_release");
                p.lightmapping = 1;
                p.lightmaptexsize = 256;
                p.lightmapquality = 50;
                p.lightao = 1;
            } else if (p.id == "ultra") {
                p.displayName = trText("profile_ultra");
                p.lightmapping = 1;
                p.lightmaptexsize = 512;
                p.lightmapquality = 100;
                p.lightao = 1;
            }
        }

        // Refresh table
        for (int r = 0; r < m_workingProfiles.size(); ++r) {
            const LightmapProfile &p = m_workingProfiles[r];

            QTableWidgetItem *item = m_profilesTable->item(r, 0);
            if (item) item->setText(p.displayName);

            QComboBox *lmCombo = qobject_cast<QComboBox*>(m_profilesTable->cellWidget(r, 1));
            if (lmCombo) lmCombo->setCurrentIndex(p.lightmapping == 1 ? 1 : 0);

            QComboBox *texCombo = qobject_cast<QComboBox*>(m_profilesTable->cellWidget(r, 2));
            if (texCombo) {
                int idx = texCombo->findData(p.lightmaptexsize);
                if (idx != -1) texCombo->setCurrentIndex(idx);
            }

            QSpinBox *qualSpin = qobject_cast<QSpinBox*>(m_profilesTable->cellWidget(r, 3));
            if (qualSpin) qualSpin->setValue(p.lightmapquality);

            QComboBox *aoCombo = qobject_cast<QComboBox*>(m_profilesTable->cellWidget(r, 4));
            if (aoCombo) aoCombo->setCurrentIndex(p.lightao == 1 ? 1 : 0);
        }
    }
}

void SettingsDialog::onSave()
{
    ConfigManager &cfg = ConfigManager::instance();
    cfg.setFpscDirectory(m_pathEdit->text().trimmed());
    cfg.setLanguage(m_languageCombo->currentData().toString());
    cfg.setAutoCleanOnSwitch(m_autoCleanCheck->isChecked());
    cfg.setAlwaysOnTop(m_alwaysOnTopCheck->isChecked());
    cfg.setShowNotifications(m_showNotificationsCheck->isChecked());
    cfg.setStartMinimized(m_startMinimizedCheck->isChecked());

    // Gather profiles from table
    for (int r = 0; r < m_workingProfiles.size(); ++r) {
        LightmapProfile &p = m_workingProfiles[r];

        QComboBox *lmCombo = qobject_cast<QComboBox*>(m_profilesTable->cellWidget(r, 1));
        if (lmCombo) p.lightmapping = lmCombo->currentData().toInt();

        QComboBox *texCombo = qobject_cast<QComboBox*>(m_profilesTable->cellWidget(r, 2));
        if (texCombo) p.lightmaptexsize = texCombo->currentData().toInt();

        QSpinBox *qualSpin = qobject_cast<QSpinBox*>(m_profilesTable->cellWidget(r, 3));
        if (qualSpin) p.lightmapquality = qualSpin->value();

        QComboBox *aoCombo = qobject_cast<QComboBox*>(m_profilesTable->cellWidget(r, 4));
        if (aoCombo) p.lightao = aoCombo->currentData().toInt();
    }

    cfg.setProfiles(m_workingProfiles);
    accept();
}
