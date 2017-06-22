#ifndef QTRCONCLIENT_H
#define QTRCONCLIENT_H

#include <QMainWindow>
#include <QDialog>
#include <QDebug>
#include <QTcpSocket>
#include <QTextStream>
#include <QLoggingCategory>
#include <QLabel>
#include <QCloseEvent>
#include <QIcon>
#include "RCon.h"

namespace Ui {
class QtRCONclient;
}

class QtRCONclient : public QMainWindow
{
    Q_OBJECT

public:
    explicit QtRCONclient(QWidget *parent = 0);
    ~QtRCONclient();
    qtRConSocketClient *rcon;
    QLabel *iconLbl;

    void closeEvent(QCloseEvent *event);

private:
    Ui::QtRCONclient *ui;

signals:
    void windowClosed();

private slots:
    void on_rconSocketError(int socketError, const QString &message);
    void on_rconNewMessage(int packetId, const QString &message);
    void on_rconCurrentStateChanged(int old_sate, int new_state);
    void on_connect_pushButton_clicked();
    void on_disconnect_pushButton_clicked();
    void on_send_pushButton_clicked();
    void on_windowClosed();

};


#endif // QtRCONCLIENT_H
