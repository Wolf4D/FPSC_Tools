#ifndef LIGHTMAPPERPROGRESSDIALOG_H
#define LIGHTMAPPERPROGRESSDIALOG_H

#include <QDialog>
#include <QProcess>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>

class LightMapperProgressDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LightMapperProgressDialog(QWidget *parent = nullptr);
    ~LightMapperProgressDialog() override;

    int runProcess(const QString &exePath, const QStringList &arguments, const QString &workingDir);

private slots:
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onProcessError(QProcess::ProcessError error);
    void onCancelClicked();

private:
    void setupUi();

    QLabel *m_titleLabel;
    QLabel *m_statusLabel;
    QLabel *m_commandLabel;
    QProgressBar *m_progressBar;
    QPushButton *m_cancelBtn;

    QProcess *m_process;
    int m_exitCode;
    bool m_success;
    QString m_errorString;
};

#endif // LIGHTMAPPERPROGRESSDIALOG_H
