/* Author: Yuriy Kuzin
 */

#include "RCon.h"

qtRConSocketClient::qtRConSocketClient(QTcpSocket *parent): QTcpSocket(parent)
{

    setId(AUTHPACKETID);
    _socket = new QTcpSocket(this);
    connect(_socket, SIGNAL(connected()), this, SLOT(on_socketConnected()));
    connect(_socket, SIGNAL(disconnected()), this, SLOT(on_socketDisconnected()));
    connect(_socket, SIGNAL(aboutToClose()), this, SLOT(on_socketAboutToClose()));
    connect(_socket, SIGNAL(readyRead()), this, SLOT(on_rconDataComing()));
    connect(_socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(on_socketError(QAbstractSocket::SocketError)));
    connect(this, SIGNAL(rconCurrentStateChanged(int, int)), this, SLOT(on_rconCurrentStateChanged(int, int)));

}
void qtRConSocketClient::on_socketError(QAbstractSocket::SocketError socketError)
{
    qDebug() << "on_socketError:" << socketError;
    emit rconSocketError(_socket->error(), _socket->errorString());
    setRconSocketLastError(_socket->error());
    //on any error if no socket connection set rcon state disconnected
    if (_socket->state()!=QAbstractSocket::ConnectedState)
        {
            setRconCurrentState(rconState::Disconnected);
        }

}


// connect to the server
void qtRConSocketClient::rconConnect(QString &address, QString password_str)
{
    setPassword(password_str);
    if (getRconCurrentState() != rconState::Connected)
        {

            setRconCurrentState(rconState::Connecting);


    _socket->open(QTcpSocket::ReadWrite);
    if (address.contains(":"))
        {
            QString addr = address.split(":").first();
            bool isnumber;
            int port = address.split(":").last().toInt(&isnumber);
            if (isnumber)
                {
                    _socket->connectToHost(addr, port);
                }
            else
                {
                    _socket->connectToHost(addr, 27015);
                }
        }
    else
        {
            _socket->connectToHost(address, 27015);
        }
    }
}


// send a rcon packet to the server
void qtRConSocketClient::rconWrite(qint32 id, const qint32 type , QString command = QString(""))
{
    RconPacket packet;
    packet.id = id;
    packet.type = type;
    packet.bodyStr = command;
    packet.size = 10 + packet.bodyStr.length() + packet.emptyStr.length();

    _socket->write((char *)&packet.size, 4);
    _socket->write((char *)&packet.id, 4);
    _socket->write((char *)&packet.type, 4);
    _socket->write(packet.bodyStr.toUtf8(), packet.bodyStr.toUtf8().length() + 1);
    _socket->write(packet.emptyStr.toUtf8(), packet.emptyStr.toUtf8().length() + 1);
}

// send a command to the server using internal ID
void qtRConSocketClient::rconSendCommand(QString command)
{
    setId(getId()+1);
    rconWrite(getId(), RCON_SERVERDATA_EXECCOMMAND, command);
}

// send a command to the server using external id
void qtRConSocketClient::rconSendCommand(int id, QString command)
{
    rconWrite(id, RCON_SERVERDATA_EXECCOMMAND, command);
}

//on data coming from server
void qtRConSocketClient::on_rconDataComing()
{
    qint32 size = 0;

    QVector <RconPacket> packets;
    RconPacket packet;
    char *buffer;
    char nullchar = 0;
    while (_socket->bytesAvailable())
        {
            //QString temp = _socket->readLine();
            //qDebug()<<temp;
            _socket->read((char *)&size, 4);
            if (size < 10 || size > 4106)
                {
                    rconDisconnect();
                    packet.bodyStr = QString("Recieved invalid packet size ");
                    packet.bodyStr.append(size).append(QString(".\n"));
                    packets.clear();
                    packets.append(packet);
                }

            packet.size = size;
            _socket->read((char *)&packet.id, 4);
            _socket->read((char *)&packet.type, 4);
            buffer = new char[packet.size - 9];
            _socket->read(buffer, packet.size - 9);
            packet.bodyStr = QString(buffer);
            delete[] buffer;
            _socket->read(&nullchar, 1);
            packets.append(packet);

        }
    //emit message by grouping all packets
    //note it is still possible that will be few large packets
    //which you need to handle by itself
    if (getRconCurrentState() == rconState::Connected)
        {
            int previd = packets[0].id;
            QString joinedPacket = "";
            for (int i = 0; i < packets.length(); i++)
                {
                    qDebug() << "id:" << packets[i].id
                             << "size:" << packets[i].size
                             << "type:" << packets[i].type
                             << "body:" << packets[i].bodyStr;
                    if (previd == packets[i].id && i != packets.length() - 1)
                        {
                            if (packets[i].bodyStr.length() > 0)
                                {
                                    joinedPacket.append(packets[i].bodyStr);
                                }
                        }
                    if (previd == packets[i].id && i == packets.length() - 1)
                        {
                            if (packets[i].bodyStr.length() > 0)
                                {
                                    joinedPacket.append(packets[i].bodyStr);
                                }
                            if (joinedPacket != "")
                                {
                                    emit rconNewMessage(previd, joinedPacket);
                                }
                        }
                    else if (previd != packets[i].id && i != packets.length() - 1)
                        {
                            if (joinedPacket != "")
                                {
                                    emit rconNewMessage(previd, joinedPacket);
                                }
                            previd = packets[i].id;
                            joinedPacket = "";
                            if (packets[i].bodyStr.length() > 0)
                                {
                                    joinedPacket.append(packets[i].bodyStr);
                                }
                        }
                    else if (previd != packets[i].id && i == packets.length() - 1)
                        {
                            emit rconNewMessage(previd, joinedPacket);
                            previd = packets[i].id;
                            joinedPacket = "";
                            if (packets[i].bodyStr.length() > 0)
                                {

                                    emit rconNewMessage(previd, packets[i].bodyStr);
                                }
                        }

                }
//            for (int i = 0; i < packets.length(); i++)
//                {
//                    qDebug() << "id:" << packets[i].id
//                             << "size:" << packets[i].size
//                             << "type:" << packets[i].type
//                             << "body:" << packets[i].bodyStr;
//                    if (packets[i].bodyStr.length() > 0)
//                        {
//                    emit rconNewMessage(packets[i].id, packets[i].bodyStr);
//                    }
//            }
        }



    else
        {
            if (packets.length() == 1 && packets[0].size == 10 && packets[0].id == AUTHPACKETID && packets[0].type == RCON_SERVERDATA_AUTH_RESPONSE && packets[0].bodyStr == "")
                {
                    qDebug() << "Autorization succeded!";
                    setRconCurrentState(rconState::Connected);
                    setRconSocketLastError(socketError::RconSocketNoError);
                }
            else if (packets.length() == 1 && packets[0].size == 10 && packets[0].id == -1 && packets[0].type == RCON_SERVERDATA_AUTH_RESPONSE && packets[0].bodyStr == "")
                {
                    qDebug() << "Autorization failed!";
                    if (packets.count() < 1)
                        {
                            emit rconSocketError(socketError::RconSocketProtocolUnsupported, "Protocol unsupported! Maybe not rcon.");
                            setRconSocketLastError(socketError::RconSocketProtocolUnsupported);
                        }
                    else
                        {
                            if (packets.last().id == -1)
                                {
                                    //not sure that this is usefull because I'm not gettin response from server in case of ban
                                    //current  behavior what I see now it's simply close connection RemoteHostClosedError
                                    if (packets[0].bodyStr.indexOf(QString("ban")) == -1)
                                        {

                                            emit rconSocketError(socketError::RconSocketAuthError, "Authorization failed! Wrong password.");
                                            setRconSocketLastError(socketError::RconSocketAuthError);
                                        }
                                    else
                                        {
                                            emit rconSocketError(socketError::RconSocketBanError, "Authorization failed! Seems you are banned.");
                                            setRconSocketLastError(socketError::RconSocketBanError);
                                        }
                                }
                        }
                    rconDisconnect();
                }
        }

}

//convert rconstate id to text value
QString qtRConSocketClient::getRconStateText(int _state)
{
    const QMetaObject metaObject = qtRConSocketClient::staticMetaObject;
    int enumIndex = metaObject.indexOfEnumerator("rconState");
    if (enumIndex == -1)
        {
            return "undefined";
        }
    QMetaEnum metaenum = metaObject.enumerator(enumIndex);
    return QString(metaenum.valueToKey(_state));

}

//slot to execute after tcp socket connected
void qtRConSocketClient::on_socketConnected()
{
    qDebug() << "Connected to socket";
    setRconCurrentState(rconState::Authorizing);
    //start autorization
    rconWrite(AUTHPACKETID, RCON_SERVERDATA_AUTH, getPassword().toUtf8());
}

//slot to execute on ech rcon state change
//here will be some reconnection mechanism
void qtRConSocketClient::on_rconCurrentStateChanged(int old_state, int new_state)
{

    qDebug() << "old_state: " << getRconStateText(old_state) << " -> new_state" << getRconStateText(new_state);
    if (new_state == rconState::Connecting)
        {

        }
    //some kind ofway to identify when you banned means server dropped socket
    else if(old_state==rconState::Authorizing && new_state==rconState::Disconnected && getRconSocketLastError()==qtRConSocketClient::RemoteHostClosedError)
    {
        emit rconSocketError(socketError::RconSocketBanError, "Authorization failed! Seems you are banned.");
        setRconSocketLastError(socketError::RconSocketBanError);
    }
}

qtRConSocketClient::~qtRConSocketClient()
{
    _socket->deleteLater();

}

//slot disconnect socket from server
void qtRConSocketClient::rconDisconnect()
{
    _socket->abort();

}

//slot disconnection started
void qtRConSocketClient::on_socketAboutToClose()
{
    qDebug() << "Disconnecting";
    setRconCurrentState(rconState::Disconnecting);
}

//slot execute when disconnected
//note it's not every time raised on some error
void qtRConSocketClient::on_socketDisconnected()
{
    qDebug() << "Disconnected";
    setRconCurrentState(rconState::Disconnected);
}
