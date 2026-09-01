#include <QApplication>
#include "core/AppController.h"

int main(int argc, char *argv[])
{
    // Enable High DPI scaling
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    QApplication a(argc, argv);
    a.setApplicationName("FPSC Tools");
    a.setApplicationDisplayName("FPSC Tools");
    a.setApplicationVersion("1.0.0 beta");
    a.setOrganizationName("Madness Studio");
    a.setOrganizationDomain("madness-studio.org");
    a.setWindowIcon(QIcon(":/app.ico"));

    // Don't quit when windows hide, allow running in tray
    a.setQuitOnLastWindowClosed(false);

    AppController controller;
    controller.start();

    return a.exec();
}
