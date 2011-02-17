#include <QtGui/QApplication>
#include <QTranslator>
#include "mainwindow.h"
#include "settingschecker.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QTranslator translator;
    translator.load("qt_ar", ":/");
    app.installTranslator(&translator);

    SettingsChecker check;
    check.checkIndexes();

    MainWindow w;
    w.show();

    w.loadIndexesList();
    w.haveIndexesCheck();

    return app.exec();
}
