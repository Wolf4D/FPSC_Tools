#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wchar.h>
#include <stdlib.h>
#include <stdint.h>
#include <string>
#include <vector>

static void createDirectories(const std::wstring &path) {
    size_t pos = 0;
    while ((pos = path.find_first_of(L"/\\", pos + 1)) != std::wstring::npos) {
        std::wstring sub = path.substr(0, pos);
        CreateDirectoryW(sub.c_str(), NULL);
    }
    CreateDirectoryW(path.c_str(), NULL);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nShowCmd) {
    (void)hPrevInstance;
    (void)lpCmdLine;

    // 1. Locate embedded payload resource
    HRSRC hRes = FindResourceW(hInstance, L"IDR_PAYLOAD", L"BIN_DATA");
    if (!hRes) {
        MessageBoxW(NULL, L"Failed to locate internal runtime payload.", L"FPSC Tools Error", MB_ICONERROR);
        return 1;
    }

    HGLOBAL hMem = LoadResource(hInstance, hRes);
    DWORD resSize = SizeofResource(hInstance, hRes);
    const uint8_t *resData = (const uint8_t*)LockResource(hMem);
    if (!resData || resSize == 0) {
        MessageBoxW(NULL, L"Failed to load internal runtime payload.", L"FPSC Tools Error", MB_ICONERROR);
        return 1;
    }

    // 2. Determine target runtime directory: %LOCALAPPDATA%/FPSC_Tools/runtime_v100_beta/
    wchar_t localAppPath[260];
    memset(localAppPath, 0, sizeof(localAppPath));
    DWORD pathLen = GetEnvironmentVariableW(L"LOCALAPPDATA", localAppPath, 260);
    if (pathLen == 0 || pathLen >= 260) {
        GetTempPathW(260, localAppPath);
    }

    std::wstring targetDir = std::wstring(localAppPath) + L"\\FPSC_Tools\\runtime_v100_beta\\";
    createDirectories(targetDir);

    std::wstring mainAppExe = targetDir + L"FPSC_Tools_app.exe";
    std::wstring stampFile = targetDir + L"version.stamp";

    // 3. Fast-path: Check if already unpacked and valid
    bool needExtract = true;
    HANDLE hStamp = CreateFileW(stampFile.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hStamp != INVALID_HANDLE_VALUE) {
        DWORD storedSize = 0;
        DWORD read = 0;
        if (ReadFile(hStamp, &storedSize, sizeof(storedSize), &read, NULL) && read == sizeof(storedSize)) {
            if (storedSize == resSize && GetFileAttributesW(mainAppExe.c_str()) != INVALID_FILE_ATTRIBUTES) {
                needExtract = false;
            }
        }
        CloseHandle(hStamp);
    }

    // 4. Extract runtime package from memory (Direct Binary Stream)
    if (needExtract) {
        const uint8_t *ptr = resData;
        const uint8_t *end = ptr + resSize;

        // Check magic header 'FPSC' (0x46505343)
        if (ptr + sizeof(uint32_t) <= end && *(const uint32_t*)ptr == 0x46505343) {
            ptr += sizeof(uint32_t);
        }

        while (ptr + sizeof(uint32_t) <= end) {
            uint32_t nameLen = *(const uint32_t*)ptr;
            ptr += sizeof(uint32_t);

            if (nameLen == 0 || ptr + nameLen > end) break;

            std::string fileName((const char*)ptr, nameLen);
            ptr += nameLen;

            if (ptr + sizeof(uint32_t) > end) break;
            uint32_t fileSize = *(const uint32_t*)ptr;
            ptr += sizeof(uint32_t);

            if (ptr + fileSize > end) break;
            const uint8_t *fileData = ptr;
            ptr += fileSize;

            std::wstring relName(fileName.begin(), fileName.end());
            for (size_t i = 0; i < relName.size(); ++i) {
                if (relName[i] == L'/') relName[i] = L'\\';
            }
            std::wstring fullPath = targetDir + relName;

            size_t lastSlash = fullPath.find_last_of(L'\\');
            if (lastSlash != std::wstring::npos) {
                createDirectories(fullPath.substr(0, lastSlash));
            }

            HANDLE hOut = CreateFileW(fullPath.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
            if (hOut != INVALID_HANDLE_VALUE) {
                DWORD written = 0;
                WriteFile(hOut, fileData, fileSize, &written, NULL);
                CloseHandle(hOut);
            }
        }

        // Write verification stamp
        HANDLE hStampOut = CreateFileW(stampFile.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hStampOut != INVALID_HANDLE_VALUE) {
            DWORD written = 0;
            WriteFile(hStampOut, &resSize, sizeof(resSize), &written, NULL);
            CloseHandle(hStampOut);
        }
    }

    // 5. Setup environment so extracted DLLs are always found
    SetDllDirectoryW(targetDir.c_str());

    std::vector<wchar_t> pathBuffer(32768, L'\0');
    DWORD curPathLen = GetEnvironmentVariableW(L"PATH", pathBuffer.data(), 32768);
    std::wstring combinedPath = targetDir;
    if (curPathLen > 0) {
        combinedPath += L";";
        combinedPath += pathBuffer.data();
    }
    SetEnvironmentVariableW(L"PATH", combinedPath.c_str());

    // Forward current working directory & arguments to the extracted app
    wchar_t currentDir[MAX_PATH];
    GetCurrentDirectoryW(MAX_PATH, currentDir);

    std::wstring cmdLine = L"\"" + mainAppExe + L"\"";
    LPWSTR fullArgs = GetCommandLineW();
    if (fullArgs) {
        // Skip argv[0] from command line
        bool inQuotes = false;
        const wchar_t *p = fullArgs;
        while (*p) {
            if (*p == L'"') inQuotes = !inQuotes;
            else if (*p == L' ' && !inQuotes) {
                while (*p == L' ') p++;
                if (*p) {
                    cmdLine += L" ";
                    cmdLine += p;
                }
                break;
            }
            p++;
        }
    }

    STARTUPINFOW si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = nShowCmd;

    PROCESS_INFORMATION pi;
    ZeroMemory(&pi, sizeof(pi));

    std::vector<wchar_t> cmdBuf(cmdLine.begin(), cmdLine.end());
    cmdBuf.push_back(L'\0');

    if (CreateProcessW(NULL, cmdBuf.data(), NULL, NULL, FALSE, 0, NULL, currentDir, &si, &pi)) {
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
        return 0;
    } else {
        MessageBoxW(NULL, L"Failed to start FPSC Tools runtime process.", L"FPSC Tools Error", MB_ICONERROR);
        return 1;
    }
}

