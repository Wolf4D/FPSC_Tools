@echo off
setlocal enabledelayedexpansion

echo ========================================================
echo   Building FPSC Tools Single-File (Qt 5.15.2 MinGW 32-bit)
echo ========================================================

set QT_DIR=C:\Qt5\5.15.2\mingw81_32
set MINGW_DIR=C:\Qt5\Tools\mingw810_32
set PATH=%QT_DIR%\bin;%MINGW_DIR%\bin;%PATH%

set BUILD_DIR=%~dp0build-release
set DIST_DIR=%~dp0dist

if not exist "%QT_DIR%\bin\qmake.exe" (
    echo [ERROR] qmake.exe not found in %QT_DIR%\bin
    exit /b 1
)

if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"
if not exist "%DIST_DIR%" mkdir "%DIST_DIR%"

echo [1/5] Compiling Qt Linguist translations with lrelease...
if exist "%QT_DIR%\bin\lrelease.exe" (
    "%QT_DIR%\bin\lrelease.exe" "%~dp0src\translations\fpsc_tool_ru.ts" -qm "%~dp0src\translations\fpsc_tool_ru.qm"
)

cd /d "%BUILD_DIR%"

echo [2/5] Running qmake...
"%QT_DIR%\bin\qmake.exe" "%~dp0src\src.pro" -spec win32-g++ "CONFIG+=release"
if %ERRORLEVEL% neq 0 (
    echo [ERROR] qmake failed!
    exit /b %ERRORLEVEL%
)

echo [3/5] Compiling Qt Application with mingw32-make...
"%MINGW_DIR%\bin\mingw32-make.exe" -j4
if %ERRORLEVEL% neq 0 (
    echo [ERROR] Compilation failed!
    exit /b %ERRORLEVEL%
)

echo [4/5] Packaging single-file runtime payload (payload.bin)...
powershell -NoProfile -Command ^
  "$bin = '%BUILD_DIR%\payload.bin';" ^
  "if (Test-Path $bin) { Remove-Item $bin -Force };" ^
  "$stream = [System.IO.File]::Create($bin);" ^
  "$writer = New-Object System.IO.BinaryWriter($stream);" ^
  "$writer.Write([uint32]0x46505343);" ^
  "function Add-F($f, $e) { if (Test-Path $f) { $bytes = [System.IO.File]::ReadAllBytes($f); $rel = [System.Text.Encoding]::UTF8.GetBytes($e); $writer.Write([uint32]$rel.Length); $writer.Write($rel); $writer.Write([uint32]$bytes.Length); $writer.Write($bytes); } };" ^
  "Add-F '%BUILD_DIR%\release\FPSC_Tools_app.exe' 'FPSC_Tools_app.exe';" ^
  "Add-F '%QT_DIR%\bin\Qt5Core.dll' 'Qt5Core.dll';" ^
  "Add-F '%QT_DIR%\bin\Qt5Gui.dll' 'Qt5Gui.dll';" ^
  "Add-F '%QT_DIR%\bin\Qt5Widgets.dll' 'Qt5Widgets.dll';" ^
  "Add-F '%QT_DIR%\bin\Qt5Concurrent.dll' 'Qt5Concurrent.dll';" ^
  "Add-F '%QT_DIR%\bin\libgcc_s_dw2-1.dll' 'libgcc_s_dw2-1.dll';" ^
  "Add-F '%MINGW_DIR%\bin\libgcc_s_dw2-1.dll' 'libgcc_s_dw2-1.dll';" ^
  "Add-F '%QT_DIR%\bin\libstdc++-6.dll' 'libstdc++-6.dll';" ^
  "Add-F '%MINGW_DIR%\bin\libstdc++-6.dll' 'libstdc++-6.dll';" ^
  "Add-F '%QT_DIR%\bin\libwinpthread-1.dll' 'libwinpthread-1.dll';" ^
  "Add-F '%MINGW_DIR%\bin\libwinpthread-1.dll' 'libwinpthread-1.dll';" ^
  "Add-F '%QT_DIR%\plugins\platforms\qwindows.dll' 'platforms/qwindows.dll';" ^
  "Add-F '%QT_DIR%\plugins\styles\qwindowsvistastyle.dll' 'styles/qwindowsvistastyle.dll';" ^
  "Add-F '%QT_DIR%\plugins\imageformats\qico.dll' 'imageformats/qico.dll';" ^
  "Add-F '%QT_DIR%\plugins\imageformats\qjpeg.dll' 'imageformats/qjpeg.dll';" ^
  "$writer.Write([uint32]0);" ^
  "$writer.Dispose();" ^
  "$stream.Dispose();"
if %ERRORLEVEL% neq 0 (
    echo [ERROR] Packaging payload failed!
    exit /b %ERRORLEVEL%
)

echo [5/5] Compiling Standalone Single-File FPSC_Tools.exe...
cd /d "%BUILD_DIR%"
"%MINGW_DIR%\bin\windres.exe" -i "%~dp0src\launcher\launcher.rc" -o launcher_res.o --include-dir="%~dp0src" --include-dir="%BUILD_DIR%"
if %ERRORLEVEL% neq 0 (
    echo [ERROR] Resource compilation failed!
    exit /b %ERRORLEVEL%
)

"%MINGW_DIR%\bin\g++.exe" -O2 -s -static -static-libgcc -static-libstdc++ -mwindows -municode "%~dp0src\launcher\main_launcher.cpp" launcher_res.o -o "%DIST_DIR%\FPSC_Tools.exe" -lshlwapi -lkernel32 -luser32 -lshell32
if %ERRORLEVEL% neq 0 (
    echo [ERROR] Single-file launcher compilation failed!
    exit /b %ERRORLEVEL%
)

:: Clean up old dlls / subdirectories in dist if they exist, leaving ONLY the single executable
if exist "%DIST_DIR%\platforms" rmdir /s /q "%DIST_DIR%\platforms"
if exist "%DIST_DIR%\styles" rmdir /s /q "%DIST_DIR%\styles"
if exist "%DIST_DIR%\imageformats" rmdir /s /q "%DIST_DIR%\imageformats"
del /q "%DIST_DIR%\*.dll" 2>nul

echo ========================================================
echo   Build Successful! 
echo   Standalone Single-File Executable: %DIST_DIR%\FPSC_Tools.exe
echo ========================================================
