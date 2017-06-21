/* Author: Yuriy Kuzin
 */

#ifndef RCON_H_INCLUDED
#define RCON_H_INCLUDED

#include <QtGlobal>
#include <QtNetwork>
#include <QUrl>
#include <QTcpSocket>


// rcon protocol described at
// http://developer.valvesoftware.com/wiki/Source_RCON_Protocol

const qint32 RCON_SERVERDATA_AUTH = 3;
const qint32 RCON_SERVERDATA_AUTH_RESPONSE = 2;
const qint32 RCON_SERVERDATA_EXECCOMMAND = 2;
const qint32 RCON_SERVERDATA_RESPONSE_VALUE = 0;

const int PACKETREADTIMEOUT = 1;
const int GENERALTIMEOUT = 1;

//just some radom number to check authorization
const int AUTHPACKETID = 33;




// rcon packet struct
typedef struct
{
    qint32 size;
    qint32 id;
    qint32 type;
    QString bodyStr;
    QString emptyStr;
} RconPacket;


class qtRConSocketClient : public QTcpSocket
{
    Q_OBJECT

    Q_PROPERTY(int rconCurrentState
               READ getRconCurrentState
               WRITE setRconCurrentState
              )
    Q_PROPERTY(QString password
               READ getPassword
               WRITE setPassword
              )
    Q_PROPERTY(int rconSocketLastError
               READ getRconSocketLastError
               WRITE setRconSocketLastError
              )
    Q_PROPERTY(qint32 Id
               READ getRconSocketLastError
               WRITE setRconSocketLastError
              )

    // add rcon errors to enum
    enum socketError { RconSocketNoError = QAbstractSocket::TemporaryError + 1,
                       RconSocketAuthError,
                       RconSocketProtocolUnsupported,
                       RconSocketBanError
                     };


public:
    //rconstate maybe some of them are odd
    //and maybe it's not the best way to describe rcon states
    //but from my point of view it'sgood as only authorization
    //is the state when you really connected to rcon
    enum rconState
    {
        Connecting,   //after send request to opentcp socket
        Authorizing,  //after send request to authorize
        Connected,    //after succesfull autorization
        Disconnecting,//after attempt to close socket
        Disconnected  //actually when socket closed
    };
    Q_ENUM(rconState)

    explicit qtRConSocketClient(QTcpSocket *parent = Q_NULLPTR);

    QTcpSocket *_socket;

    QVector<RconPacket> ArrayOfRConPackets;

    void rconConnect(QString &address, QString passwordstr);
    bool rconIsConnected() { return rconCurrentState == rconState::Connected; }
    void rconSendCommand(int id, QString command);
    void rconSendCommand(QString command);
    void rconDisconnect();

    ~qtRConSocketClient();

    void setRconCurrentState(int m_rconCurrentState)
    {
        rconOldState = rconCurrentState;
        rconCurrentState = m_rconCurrentState;
        emit rconCurrentStateChanged(rconOldState, rconCurrentState);
    }
    int getRconCurrentState() const
    { return rconCurrentState; }

    void setPassword(QString m_password)
    {
        password = m_password;
    }
    QString getPassword() const
    { return password; }

    void setRconSocketLastError(int m_rconSocketLastError)
    {
        rconSocketLastError = m_rconSocketLastError;
    }
    int getRconSocketLastError() const
    { return rconSocketLastError; }

    void setId(int m_Id)
    {
        Id = m_Id;
    }
    int getId() const
    { return Id; }

    QString getRconStateText(int _state);


private:
    qint32 Id;
    QString password = "";
    int rconCurrentState = rconState::Disconnected;
    int rconOldState = rconState::Disconnected;
    int rconSocketLastError = socketError::RconSocketNoError;


protected:
    //QVector <RconPacket> PacketRead();
    qint32 rconWrite(qint32 id, qint32 type , QString command);

private slots:
    void on_rconDataComing();
    void on_socketConnected();
    void on_socketDisconnected();
    void on_socketAboutToClose();
    void on_socketError(QAbstractSocket::SocketError socketError);
    void on_rconCurrentStateChanged(int old_sate, int new_state);
signals:
    void rconNewMessage(int packetId, const QString &message);
    void rconSocketError(int socketError, const QString &message);
    void rconCurrentStateChanged(int old_state, int new_state);

};

#endif // RCON_H_INCLUDED
