#ifndef PROCESSCONTROLLER_H
#define PROCESSCONTROLLER_H

#include <QObject>
#include <QString>
#include <QStringList>

class ProcessController : public QObject
{
    Q_OBJECT

public:
    static ProcessController& instance();

    int killFPSC();
    bool launchFPSC(const QString &fpscDir);
    bool restartFPSC(const QString &fpscDir);
    bool isFPSCRunning() const;
    QStringList runningProcesses() const;
    QString findLightMapperExe(const QString &fpscDir) const;
    QString findCurrentMapFpmPath(const QString &fpscDir, QString *outRelativePath = nullptr) const;

signals:
    void processesKilled(int count);
    void processLaunched(const QString &exePath, bool success);
    void fpscStatusChanged(bool isRunning);

private:
    explicit ProcessController(QObject *parent = nullptr);
    ~ProcessController() override = default;
    ProcessController(const ProcessController&) = delete;
    ProcessController& operator=(const ProcessController&) = delete;

    QStringList targetProcessNames() const;
};

#endif // PROCESSCONTROLLER_H
