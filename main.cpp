#include <QtGui/QApplication>
#include "mainwindow.h"

#define APP_VERSION "0.6"
#define APP_NAME "CLucene Test - " APP_VERSION

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.setWindowTitle(APP_NAME);

    w.show();
    return a.exec();
}
