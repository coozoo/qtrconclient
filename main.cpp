#include "qtrconclient.h"
#include <QApplication>

void customLogHandler(QtMsgType type, const QMessageLogContext& context,
                      const QString& msg)
{
    Q_UNUSED(context);
    QTextStream cout(stdout);
    QTextStream cerr(stderr);
    if(type!=QtCriticalMsg || type!=QtFatalMsg)
    {
        cout<<msg<<endl;
    }
    else
    {
        cerr<<msg<<endl;
    }
}

int main(int argc, char *argv[])
{
    qInstallMessageHandler(&customLogHandler);
    QApplication a(argc, argv);
    QString platform="";
    Q_UNUSED(platform);
    #if __GNUC__
        #if __x86_64__
            platform="-64bit";
        #endif
    #endif

    a.setProperty("appversion","0.1b" + platform);
    a.setProperty("appname","Qt Rcon Client");

    QtRCONclient w;
    w.setWindowTitle(a.property("appname").toString() + " " + a.property("appversion").toString());
    w.show();

    return a.exec();
}
