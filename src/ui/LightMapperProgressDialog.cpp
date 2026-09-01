#include "LightMapperProgressDialog.h"
#include "Theme.h"
#include "I18n.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QApplication>
#include <QFileInfo>
#include <QDir>

LightMapperProgressDialog::LightMapperProgressDialog(QWidget *parent)
    : QDialog(parent)
    , m_process(nullptr)
    , m_exitCode(-1)
    , m_success(false)
{
    setWindowTitle("LightMapper");
    setStyleSheet(Theme::darkStyleSheet());
    setFixedSize(420, 180);
    setModal(true);
    setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::CustomizeWindowHint);

    setupUi();
}

LightMapperProgressDialog::~LightMapperProgressDialog()
{
    if (m_process && m_process->state() != QProcess::NotRunning) {
        m_process->kill();
        m_process->waitForFinished(500);
    }
}

void LightMapperProgressDialog::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 18, 20, 18);
    mainLayout->setSpacing(10);

    m_titleLabel = new QLabel(QString("<b>%1</b>").arg(trText("lightmapper_wait_title")), this);
    m_titleLabel->setStyleSheet("font-size: 14px; color: #00bcd4;");
    mainLayout->addWidget(m_titleLabel);

    m_statusLabel = new QLabel(trText("lightmapper_wait_msg"), this);
    m_statusLabel->setStyleSheet("color: #e0e6ed; font-size: 12px;");
    m_statusLabel->setWordWrap(true);
    mainLayout->addWidget(m_statusLabel);

    m_commandLabel = new QLabel("<code>LightMapper.exe BIMlightmapper</code>", this);
    m_commandLabel->setStyleSheet("color: #8a99ad; font-size: 11px; background-color: #1a1c22; padding: 4px 6px; border-radius: 4px; border: 1px solid #2d3139;");
    mainLayout->addWidget(m_commandLabel);

    m_progressBar = new QProgressBar(this);
    m_progressBar->setRange(0, 0); // Indeterminate animated bar
    m_progressBar->setFixedHeight(12);
    m_progressBar->setTextVisible(false);
    m_progressBar->setStyleSheet("QProgressBar { background-color: #1e2026; border: 1px solid #2d3139; border-radius: 6px; } QProgressBar::chunk { background-color: #00bcd4; border-radius: 5px; }");
    mainLayout->addWidget(m_progressBar);

    mainLayout->addSpacing(4);

    QHBoxLayout *btnRow = new QHBoxLayout();
    btnRow->addStretch();

    m_cancelBtn = new QPushButton(trText("settings_cancel"), this);
    m_cancelBtn->setFixedWidth(90);
    connect(m_cancelBtn, &QPushButton::clicked, this, &LightMapperProgressDialog::onCancelClicked);
    btnRow->addWidget(m_cancelBtn);

    mainLayout->addLayout(btnRow);
}

int LightMapperProgressDialog::runProcess(const QString &exePath, const QStringList &arguments, const QString &workingDir)
{
    QString cmdText = QString("<code>%1 %2</code>").arg(QFileInfo(exePath).fileName()).arg(arguments.join(" "));
    m_commandLabel->setText(cmdText);
    m_commandLabel->setToolTip(QString("Working directory: %1\nExecutable: %2").arg(workingDir).arg(exePath));

    m_process = new QProcess(this);
    m_process->setWorkingDirectory(workingDir);

    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &LightMapperProgressDialog::onProcessFinished);
    connect(m_process, &QProcess::errorOccurred,
            this, &LightMapperProgressDialog::onProcessError);

    m_process->start(exePath, arguments);

    if (!m_process->waitForStarted(3000)) {
        m_exitCode = -1;
        m_success = false;
        return -1;
    }

    exec();

    return m_exitCode;
}

void LightMapperProgressDialog::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    m_exitCode = exitCode;
    m_success = (exitStatus == QProcess::NormalExit && exitCode == 0);
    accept();
}

void LightMapperProgressDialog::onProcessError(QProcess::ProcessError error)
{
    Q_UNUSED(error);
    m_exitCode = -1;
    m_success = false;
    reject();
}

void LightMapperProgressDialog::onCancelClicked()
{
    if (m_process && m_process->state() != QProcess::NotRunning) {
        m_process->kill();
        m_process->waitForFinished(500);
    }
    m_exitCode = -999; // Cancelled by user
    m_success = false;
    reject();
}
