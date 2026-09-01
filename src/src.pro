QT += core gui widgets concurrent

CONFIG += c++11 release

TARGET = FPSC_Tools_app
TEMPLATE = app

INCLUDEPATH += $$PWD \
               $$PWD/core \
               $$PWD/ui

HEADERS += \
    core/I18n.h \
    core/ConfigManager.h \
    core/ProcessController.h \
    core/SetupIniManager.h \
    core/FastCleaner.h \
    core/StashManager.h \
    core/AppController.h \
    ui/FloatingToolbar.h \
    ui/MiniIconWidget.h \
    ui/TrayManager.h \
    ui/SettingsDialog.h \
    ui/StashDialog.h \
    ui/LightMapperProgressDialog.h \
    ui/Theme.h

SOURCES += \
    main.cpp \
    core/I18n.cpp \
    core/ConfigManager.cpp \
    core/ProcessController.cpp \
    core/SetupIniManager.cpp \
    core/FastCleaner.cpp \
    core/StashManager.cpp \
    core/AppController.cpp \
    ui/FloatingToolbar.cpp \
    ui/MiniIconWidget.cpp \
    ui/TrayManager.cpp \
    ui/SettingsDialog.cpp \
    ui/StashDialog.cpp \
    ui/LightMapperProgressDialog.cpp

RESOURCES += resources.qrc

TRANSLATIONS += translations/fpsc_tool_ru.ts

RC_FILE = app.rc
