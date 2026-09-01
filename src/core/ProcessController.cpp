#include "ProcessController.h"
#include <QProcess>
#include <QDir>
#include <QFileInfo>
#include <QThread>
#include <QDebug>

#ifdef Q_OS_WIN
#include <windows.h>
#include <tlhelp32.h>
#endif

ProcessController& ProcessController::instance()
{
    static ProcessController s_instance;
    return s_instance;
}

ProcessController::ProcessController(QObject *parent)
    : QObject(parent)
{
}

QStringList ProcessController::targetProcessNames() const
{
    return {
        "FPS Creator.exe",
        "FPS Creator Mapeditor.exe",
        "MapEditor.exe",
        "FPSCreator.exe",
        "FPSC-Game.exe",
        "FPSC-MapEditor.exe",
        "FPSC-V109.exe",
        "FPSC-V118.exe",
        "FPSC-V119.exe",
        "FPSC-V120.exe",
        "FPSC.exe"
    };
}

int ProcessController::killFPSC()
{
    int killedCount = 0;

#ifdef Q_OS_WIN
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return 0;
    }

    PROCESSENTRY32W pe;
    pe.dwSize = sizeof(PROCESSENTRY32W);

    QStringList targets = targetProcessNames();

    if (Process32FirstW(hSnapshot, &pe)) {
        do {
            QString exeName = QString::fromWCharArray(pe.szExeFile);
            for (const QString &target : targets) {
                if (exeName.compare(target, Qt::CaseInsensitive) == 0) {
                    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pe.th32ProcessID);
                    if (hProcess != NULL) {
                        if (TerminateProcess(hProcess, 0)) {
                            killedCount++;
                        }
                        CloseHandle(hProcess);
                    }
                    break;
                }
            }
        } while (Process32NextW(hSnapshot, &pe));
    }

    CloseHandle(hSnapshot);
#endif

    emit processesKilled(killedCount);
    emit fpscStatusChanged(isFPSCRunning());
    return killedCount;
}

bool ProcessController::isFPSCRunning() const
{
#ifdef Q_OS_WIN
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return false;
    }

    PROCESSENTRY32W pe;
    pe.dwSize = sizeof(PROCESSENTRY32W);

    QStringList targets = targetProcessNames();
    bool running = false;

    if (Process32FirstW(hSnapshot, &pe)) {
        do {
            QString exeName = QString::fromWCharArray(pe.szExeFile);
            for (const QString &target : targets) {
                if (exeName.compare(target, Qt::CaseInsensitive) == 0) {
                    running = true;
                    break;
                }
            }
            if (running) break;
        } while (Process32NextW(hSnapshot, &pe));
    }

    CloseHandle(hSnapshot);
    return running;
#else
    return false;
#endif
}

QStringList ProcessController::runningProcesses() const
{
    QStringList found;

#ifdef Q_OS_WIN
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return found;
    }

    PROCESSENTRY32W pe;
    pe.dwSize = sizeof(PROCESSENTRY32W);

    QStringList targets = targetProcessNames();

    if (Process32FirstW(hSnapshot, &pe)) {
        do {
            QString exeName = QString::fromWCharArray(pe.szExeFile);
            for (const QString &target : targets) {
                if (exeName.compare(target, Qt::CaseInsensitive) == 0) {
                    found.append(exeName);
                    break;
                }
            }
        } while (Process32NextW(hSnapshot, &pe));
    }

    CloseHandle(hSnapshot);
#endif

    return found;
}

bool ProcessController::launchFPSC(const QString &fpscDir)
{
    if (fpscDir.isEmpty()) {
        emit processLaunched("", false);
        return false;
    }

    QDir dir(fpscDir);
    if (!dir.exists()) {
        emit processLaunched("", false);
        return false;
    }

    // Determine executable to launch: prefer "FPS Creator.exe", then "FPS Creator Mapeditor.exe", etc.
    QStringList preferredExes = {
        "FPS Creator.exe",
        "FPS Creator Mapeditor.exe",
        "MapEditor.exe",
        "FPSCreator.exe",
        "FPSC-V120.exe",
        "FPSC-V118.exe",
        "FPSC.exe"
    };

    QString targetExePath;
    for (const QString &exe : preferredExes) {
        QString candidate = dir.filePath(exe);
        if (QFileInfo::exists(candidate)) {
            targetExePath = candidate;
            break;
        }
    }

    if (targetExePath.isEmpty()) {
        emit processLaunched("", false);
        return false;
    }

    bool started = QProcess::startDetached(targetExePath, QStringList(), dir.absolutePath());
    emit processLaunched(targetExePath, started);
    emit fpscStatusChanged(isFPSCRunning());
    return started;
}

bool ProcessController::restartFPSC(const QString &fpscDir)
{
    killFPSC();
    // Wait slightly to ensure all file locks and DirectX contexts are freed
    QThread::msleep(200);
    return launchFPSC(fpscDir);
}

QString ProcessController::findLightMapperExe(const QString &fpscDir) const
{
    if (fpscDir.isEmpty()) return "";

    QDir dir(fpscDir);
    QStringList candidates = {
        dir.filePath("LightMapper.exe"),
        dir.filePath("Files/LightMapper.exe"),
        dir.filePath("Lightmapper.exe"),
        dir.filePath("Files/Lightmapper.exe"),
        dir.filePath("Tools/LightMapper.exe"),
        dir.filePath("Files/Tools/LightMapper.exe"),
        dir.filePath("BIMlightmapper.exe"),
        dir.filePath("Files/BIMlightmapper.exe")
    };

    for (const QString &cand : candidates) {
        if (QFileInfo::exists(cand)) {
            return QFileInfo(cand).absoluteFilePath();
        }
    }

    return "";
}

#ifdef Q_OS_WIN
namespace {
struct FindFpscWindowData {
    QString foundTitle;
};

static BOOL CALLBACK EnumFpscWindowsCallback(HWND hwnd, LPARAM lParam) {
    if (!IsWindowVisible(hwnd)) return TRUE;
    wchar_t titleBuf[512];
    int len = GetWindowTextW(hwnd, titleBuf, 512);
    if (len > 0) {
        QString title = QString::fromWCharArray(titleBuf, len);
        if (title.contains("FPS Creator", Qt::CaseInsensitive) && title.contains("[") && title.contains("]")) {
            FindFpscWindowData *data = reinterpret_cast<FindFpscWindowData*>(lParam);
            data->foundTitle = title;
            return FALSE; // Stop enumerating
        }
    }
    return TRUE;
}
}
#endif

QString ProcessController::findCurrentMapFpmPath(const QString &fpscDir, QString *outRelativePath) const
{
#ifdef Q_OS_WIN
    FindFpscWindowData data;
    EnumWindows(EnumFpscWindowsCallback, reinterpret_cast<LPARAM>(&data));

    if (data.foundTitle.isEmpty()) {
        return "";
    }

    // Title format example: "FPS Creator - [mapbank\1.fpm]"
    int startIdx = data.foundTitle.indexOf('[');
    int endIdx = data.foundTitle.lastIndexOf(']');
    if (startIdx != -1 && endIdx > startIdx) {
        QString inside = data.foundTitle.mid(startIdx + 1, endIdx - startIdx - 1).trimmed();
        inside.replace('\\', '/');

        if (inside.endsWith(".fpm", Qt::CaseInsensitive)) {
            if (outRelativePath) {
                *outRelativePath = inside;
            }

            QFileInfo directFi(inside);
            if (directFi.isAbsolute() && directFi.exists()) {
                return directFi.absoluteFilePath();
            }

            if (!fpscDir.isEmpty()) {
                QDir rootDir(fpscDir);
                QString pathInFiles = rootDir.filePath("Files/" + inside);
                if (QFileInfo::exists(pathInFiles)) {
                    return QFileInfo(pathInFiles).absoluteFilePath();
                }

                QString pathInRoot = rootDir.filePath(inside);
                if (QFileInfo::exists(pathInRoot)) {
                    return QFileInfo(pathInRoot).absoluteFilePath();
                }
            }
        }
    }
#else
    Q_UNUSED(fpscDir);
    Q_UNUSED(outRelativePath);
#endif
    return "";
}
