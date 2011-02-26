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

    QString dateTime = QDateTime::currentDateTime().toString("[dd/MM/yyyy] [hh:mm:ss:zzz] ");

    switch (type) {
    case QtDebugMsg:
        out << dateTime << "[DEBUG] " << QString::fromLocal8Bit(msg) << "\n";
        break;
    case QtWarningMsg:
        out << dateTime << "[WARNING] " << QString::fromLocal8Bit(msg) << "\n";
        break;
    case QtCriticalMsg:
        out << dateTime << "[CRITICAL] " << QString::fromLocal8Bit(msg) << "\n";
        break;
    case QtFatalMsg:
        out << dateTime << "[FATAL] " << QString::fromLocal8Bit(msg) << "\n";
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

    QSettings settings(SETTINGS_FILE, QSettings::IniFormat);
    SettingsChecker check;

    if(settings.value("checkIndexes", true).toBool())
        check.checkIndexes();

    MainWindow w;
    w.show();

    w.loadIndexesList();
    w.haveIndexesCheck();

    int ret = app.exec();

    if(!ret)
        qDebug("Exit with success");
    else
        qDebug("Exit with code: %d", ret);

    return ret;
}
