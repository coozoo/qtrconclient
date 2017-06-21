#include "qtrconclient.h"
#include "ui_qtrconclient.h"

QtRCONclient::QtRCONclient(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::QtRCONclient)
{
    ui->setupUi(this);
    rcon = new qtRConSocketClient();

    iconLbl=new QLabel(ui->statusBar);
    ui->statusBar->addPermanentWidget(iconLbl,0);
    iconLbl->setPixmap(QPixmap(":/images/"+rcon->getRconStateText(qtRConSocketClient::Disconnected)+".png").scaled(QSize(20,20)));

    ui->connect_pushButton->setEnabled(true);
    ui->disconnect_pushButton->setEnabled(false);
    ui->send_pushButton->setEnabled(false);

    ui->address_lineEdit->setText("192.168.1.38");
    ui->password_lineEdit->setEchoMode(QLineEdit::Password);
    ui->password_lineEdit->setText("123");


    connect(rcon,SIGNAL(rconSocketError(int,QString)),this,SLOT(on_rconSocketError(int,QString)));
    connect(rcon,SIGNAL(rconNewMessage(int,QString)),this,SLOT(on_rconNewMessage(int,QString)));
    connect(rcon,SIGNAL(rconCurrentStateChanged(int,int)),this,SLOT(on_rconCurrentStateChanged(int,int)));
    connect(this,SIGNAL(windowClosed()),this,SLOT(on_windowClosed()));



}

QtRCONclient::~QtRCONclient()
{
    iconLbl->deleteLater();
    rcon->deleteLater();
    delete ui;
}


void QtRCONclient::on_rconSocketError(int socketError, const QString &message)
{
    qDebug()<<"SocketError:"<<socketError<<"; "<<"Message:"<<message;
    ui->output_textEdit->append("SocketError:" + QString::number(socketError,'g',10) + "; Message:" +message);
}

void QtRCONclient::on_connect_pushButton_clicked()
{
    qDebug()<<"on_connect_pushButton_clicked";
    QString address=ui->address_lineEdit->text();
    QString password=ui->password_lineEdit->text();
    rcon->rconConnect(address,password);
}

void QtRCONclient::on_disconnect_pushButton_clicked()
{
    qDebug()<<"on_disconnect_pushButton_clicked";
    rcon->rconDisconnect();
}

void QtRCONclient::on_send_pushButton_clicked()
{
    qDebug()<<"on_send_pushButton_clicked";
        if(rcon->rconIsConnected())
        {
            qDebug()<<"connected";
            rcon->rconSendCommand(ui->command_lineEdit->text());
        }
        else
        {
            qWarning()<<"rcon is closed";
        }
}

void QtRCONclient::on_rconNewMessage(int packetId, const QString &message)
{
    ui->output_textEdit->append(QString::number(rcon->getId(),'g',10));
    ui->output_textEdit->append("packetId:" + QString::number(packetId,'g',10) + "; \nMessage:" +message);
}

void QtRCONclient::on_rconCurrentStateChanged(int old_state, int new_state)
{
    ui->statusBar->showMessage(rcon->getRconStateText(new_state));
    iconLbl->setPixmap(QPixmap(":/images/" + rcon->getRconStateText(new_state) + ".png").scaled(QSize(20,20)));
    if(new_state==qtRConSocketClient::Connected)
    {
        ui->connect_pushButton->setEnabled(false);
        ui->disconnect_pushButton->setEnabled(true);
        ui->send_pushButton->setEnabled(true);
    }
    else
    {
        ui->connect_pushButton->setEnabled(true);
        ui->disconnect_pushButton->setEnabled(false);
        ui->send_pushButton->setEnabled(false);
    }
}

void QtRCONclient::closeEvent(QCloseEvent *event)
{
    emit windowClosed();
    event->accept();

}

void QtRCONclient::on_windowClosed()
{
    qDebug()<<"Closing application";
    rcon->rconDisconnect();
}
