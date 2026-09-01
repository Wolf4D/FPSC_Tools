#ifndef STASHMANAGER_H
#define STASHMANAGER_H

#include <QObject>
#include <QString>
#include <QDateTime>
#include <QVector>

struct StashInfo {
    QString id;
    QString name;
    QDateTime createdAt;
    int fileCount = 0;
    qint64 totalBytes = 0;
    QString folderPath;
    QString fpmSourcePath;
    QString fpmRelativePath;

    QString formattedBytes() const;
    QString formattedTime() const;
};

class StashManager : public QObject
{
    Q_OBJECT

public:
    static StashManager& instance();

    QString testLevelDirectory(const QString &fpscDir) const;
    QString stashesRootDirectory(const QString &fpscDir) const;

    bool quickSaveStash(const QString &fpscDir, QString *outError = nullptr);
    bool quickRestoreStash(const QString &fpscDir, QString *outError = nullptr);

    bool createStash(const QString &fpscDir, const QString &name, StashInfo *outInfo = nullptr, QString *outError = nullptr);
    bool restoreStash(const QString &fpscDir, const QString &stashId, QString *outError = nullptr);
    bool deleteStash(const QString &fpscDir, const QString &stashId, QString *outError = nullptr);
    QVector<StashInfo> listStashes(const QString &fpscDir) const;

signals:
    void stashSaved(const StashInfo &info);
    void stashRestored(const QString &stashId);
    void stashesUpdated();

private:
    explicit StashManager(QObject *parent = nullptr);
    ~StashManager() override = default;
    StashManager(const StashManager&) = delete;
    StashManager& operator=(const StashManager&) = delete;

    bool copyDirectoryRecursively(const QString &srcPath, const QString &dstPath, int &fileCount, qint64 &totalBytes);
    bool removeDirectoryContents(const QString &dirPath);
    void readStashMeta(const QString &stashDir, StashInfo &info) const;
    void writeStashMeta(const QString &stashDir, const StashInfo &info) const;
};

#endif // STASHMANAGER_H
