#include <QtGui/QApplication>
#include <QTranslator>
#include "mainwindow.h"

#define APP_VERSION "0.8"
#define APP_NAME "CLucene Test - " APP_VERSION

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QTranslator translator;
    translator.load("qt_ar", ":/data");
    app.installTranslator(&translator);

    MainWindow w;
    w.setWindowTitle(APP_NAME);

    w.show();
    return app.exec();
}
