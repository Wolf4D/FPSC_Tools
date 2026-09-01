#ifndef FASTCLEANER_H
#define FASTCLEANER_H

#include <QObject>
#include <QString>
#include <QFutureWatcher>

enum class CleanType {
    BinAndDbo,
    Lightmaps,
    All,
    LevelBuildData,
    EngineTemp
};

struct CleanResult {
    CleanType type;
    int filesDeleted = 0;
    qint64 bytesFreed = 0;
    qint64 elapsedMs = 0;
    bool success = true;
    QString errorMessage;

    QString formattedBytes() const;
    QString summary() const;
};

class FastCleaner : public QObject
{
    Q_OBJECT

public:
    static FastCleaner& instance();

    void startCleanAsync(const QString &fpscDir, CleanType type);
    static CleanResult cleanSync(const QString &fpscDir, CleanType type);

    static CleanResult cleanBinAndDbo(const QString &fpscDir);
    static CleanResult cleanLightmaps(const QString &fpscDir);
    static CleanResult cleanAll(const QString &fpscDir);
    static CleanResult cleanLevelBuildData(const QString &fpscDir);
    static CleanResult cleanEngineTemp();

signals:
    void cleanStarted(CleanType type);
    void cleanFinished(CleanType type, const CleanResult &result);

private:
    explicit FastCleaner(QObject *parent = nullptr);
    ~FastCleaner() override = default;
    FastCleaner(const FastCleaner&) = delete;
    FastCleaner& operator=(const FastCleaner&) = delete;

    QFutureWatcher<CleanResult> *m_watcher;
};

#endif // FASTCLEANER_H
