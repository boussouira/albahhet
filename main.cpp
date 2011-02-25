#include <QtGui/QApplication>
#include <QTranslator>
#include <stdlib.h>
#include "mainwindow.h"
#include "settingschecker.h"
#include <qfile.h>
#include <qtextstream.h>
#include <qtextcodec.h>
#include <qdatetime.h>

void myMessageOutput(QtMsgType type, const char *msg)
{
    QFile debugFile("log.txt");
    if (!debugFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append))
             return;

    QTextStream out(&debugFile);
    out.setCodec("utf-8");

    QDateTime time = QDateTime::currentDateTime();

    switch (type) {
    case QtDebugMsg:
        out << "[" << time.toString("hh:mm:ss:zzz") << "] "
                << "[DEBUG] " << QString::fromLocal8Bit(msg) << "\n";
        break;
    case QtWarningMsg:
        out << "[" << time.toString("hh:mm:ss:zzz") << "] "
                << "[WARNING] " << QString::fromLocal8Bit(msg) << "\n";
        break;
    case QtCriticalMsg:
        out << "[" << time.toString("hh:mm:ss:zzz") << "] "
                << "[CRITICAL] " << QString::fromLocal8Bit(msg) << "\n";
        break;
    case QtFatalMsg:
        out << "[" << time.toString("hh:mm:ss:zzz") << "] "
                << "[FATAL] " << QString::fromLocal8Bit(msg) << "\n";
        abort();
    }
}

int main(int argc, char *argv[])
{
    qInstallMsgHandler(myMessageOutput);
    qDebug("Starting the application");

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
