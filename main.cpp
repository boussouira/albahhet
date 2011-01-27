#include <QtGui/QApplication>
#include <QTranslator>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QTranslator translator;
    translator.load("qt_ar", ":/");
    app.installTranslator(&translator);

    MainWindow w;
    w.show();

    w.loadIndexesList();
    w.haveIndexesCheck();

    return app.exec();
}
