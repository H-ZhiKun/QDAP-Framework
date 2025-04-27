#include "TcpServer.h"
#include <QJsonObject>
#include <QMetaEnum>
#include <qjsondocument.h>
#include <qlogging.h>

using namespace _Kits;
TcpServer::TcpServer(QObject *parent) : QObject(parent)
{
}

TcpServer::~TcpServer() noexcept
{
}

bool TcpServer::setPort(int port)
{
    this->serverPort = port;
    if (this->serverPort == port)
        return 1;
    return 0;
}

bool TcpServer::startServer()
{
    tcpServer = new QTcpServer(this);
    connect(tcpServer,
            &QTcpServer::newConnection,
            this,
            &TcpServer::onNewConnection);
    if (!tcpServer->listen(QHostAddress::Any, serverPort))
    {
        return false;
    }
    return true;
}
bool TcpServer::stopServer()
{
    if (tcpServer)
    {
        foreach (QTcpSocket *client, clients)
        {
            client->disconnectFromHost();
            client->close();
        }
        tcpServer->close();
        tcpServer->deleteLater();
    }
    return true;
}

bool TcpServer::sendData(QTcpSocket *client, const QByteArray &data)
{
    bool ret = false;
    if (client && client->state() == QAbstractSocket::ConnectedState)
    {
        client->write(data);
        client->flush();
        ret = true;
    }
    return ret;
}

void TcpServer::onNewConnection()
{
    while (tcpServer->hasPendingConnections())
    {
        QTcpSocket *client = tcpServer->nextPendingConnection();
        clients.push_back(client);
        connect(client,
                &QTcpSocket::disconnected,
                this,
                &TcpServer::onClientDisconnected);
        connect(client,
                &QAbstractSocket::errorOccurred,
                [client, this](QAbstractSocket::SocketError socketError) {
                    qDebug() << client->peerAddress().toString()
                             << socketErrorToString(socketError);
                });
        connect(client, &QTcpSocket::readyRead, this, &TcpServer::onReadyRead);
    }
}

void TcpServer::onClientDisconnected()
{
    QTcpSocket *client = qobject_cast<QTcpSocket *>(sender());
    if (client)
    {
        clients.erase(std::remove(clients.begin(), clients.end(), client),
                      clients.end());
        client->deleteLater();
    }
}

void TcpServer::onReadyRead()
{
    QTcpSocket *client = qobject_cast<QTcpSocket *>(sender());
    if (client)
    {
        QByteArray recvData = client->readAll();
        m_buffer.append(recvData);
        while (true)
        {
            if (m_expectedSize == 0 && m_buffer.size() >= sizeof(quint32))
            {
                // 从缓冲区读取长度字段
                QDataStream stream(m_buffer);
                stream.setByteOrder(QDataStream::LittleEndian);
                stream >> m_expectedSize;
                m_buffer.remove(0, sizeof(quint32));
            }

            // 如果已读取长度字段，且缓冲区足够容纳实际数据
            if (m_expectedSize > 0 && m_buffer.size() >= m_expectedSize)
            {
                QByteArray packet = m_buffer.left(m_expectedSize);
                m_buffer.remove(0, m_expectedSize);
                m_expectedSize = 0;
                emit dataReceived(client, packet);
            }
            else
            {
                break;
            }
        }
    }
}

QString TcpServer::socketErrorToString(QAbstractSocket::SocketError socketError)
{
    const QMetaObject &metaObject = QAbstractSocket::staticMetaObject;
    int index = metaObject.indexOfEnumerator("SocketError");
    QMetaEnum metaEnum = metaObject.enumerator(index);
    const char *errorString = metaEnum.valueToKey(socketError);
    if (errorString)
    {
        return QString::fromLatin1(errorString);
    }
    else
    {
        return QString();
    }
}
