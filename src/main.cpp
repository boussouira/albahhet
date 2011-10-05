#include <QtGui/QApplication>
#include <QTranslator>
#include <stdlib.h>
#include "mainwindow.h"
#include "settingschecker.h"
#include <qfile.h>
#include <qtextstream.h>
#include <qtextcodec.h>
#include <qdatetime.h>
#include <qsettings.h>
#include <qmessagebox.h>

#ifdef Q_OS_WIN
    #include <Windows.h>
#endif


void myMessageOutput(QtMsgType type, const char *msg)
{
    QFile debugFile(LOG_FILE);
    if (!debugFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append)) {
        return;
    }

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
        QMessageBox::critical(0, QObject::tr("Fatal error"), msg);
        exit(-1);
    }

    debugFile.close();
}

#ifdef Q_OS_WIN
void useArabicKeyboardLayout()
{
    HKL local = LoadKeyboardLayout(L"00000401", KLF_ACTIVATE);

    if(local != NULL)
        ActivateKeyboardLayout(local, KLF_ACTIVATE);
    else
        qWarning("Can't load Arabic Keyboard Layout");
}
#endif

void clearLogFile()
{
    QDir dir;
    dir.mkpath(QDesktopServices::storageLocation(QDesktopServices::DataLocation));

    QFile debugFile(LOG_FILE);
    if(debugFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
        debugFile.close();
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setOrganizationName("Al Bahhet");
    app.setOrganizationDomain("albahhet.sf.net");
    app.setApplicationName("Al Bahhet");
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QTextCodec::setCodecForTr(QTextCodec::codecForName("utf-8"));

    clearLogFile();
//    qInstallMsgHandler(myMessageOutput);
    qDebug("Starting the application");

    QTranslator translator;
    translator.load("qt_ar", ":/");
    app.installTranslator(&translator);

    QSettings settings;
    SettingsChecker check;

    if(settings.value("checkIndexes", true).toBool())
        check.checkIndexes();

#ifdef Q_OS_WIN
    useArabicKeyboardLayout();
#endif

    MainWindow w;
    w.show();

    w.loadIndexesList();
    w.haveIndexesCheck();

    QObject::connect(&app, SIGNAL(lastWindowClosed()), &app, SLOT(quit()));

     return app.exec();
}
