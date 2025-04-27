#include "TcpClient.h"
#include <qtimer.h>
namespace _Kits
{
    TcpClient::TcpClient(const QString &host,
                         quint16 port,
                         int reconnectSeconds,
                         int heartSeconds,
                         QString heartContent,
                         QObject *parent)
        : QObject(parent), host_(host), port_(port),
          reconnectSeconds_(reconnectSeconds), heartSeconds_(heartSeconds),
          heartContent_(heartContent.toUtf8())
    {
    }

    TcpClient::~TcpClient() noexcept
    {
        if (pingTimer_ != nullptr)
        {
            if (pingTimer_->isActive())
            pingTimer_->stop();
        }
        
        if (reconnectTimer_ != nullptr)
        {
            if (reconnectTimer_->isActive())
                reconnectTimer_->stop();
        }

        if (tcp_->state() == QAbstractSocket::ConnectedState)
        {
            qDebug() << "Disconnecting from host...";
            tcp_->disconnectFromHost();

            // 等待对端确认断开（可选）
            if (!tcp_->waitForDisconnected(3000))
            {
                qDebug() << "Disconnect timeout. Aborting...";
                tcp_->abort(); // 超时后强制断开
            }
            else
            {
                qDebug() << "TCP Disconnected successfully.";
            }
        }
    }

    bool TcpClient::start()
    {
        tcp_ = new QTcpSocket(this);
        if (heartSeconds_ > 0)
        {
            pingTimer_ = new QTimer(this);
            pingTimer_->setInterval(heartSeconds_ * 1000);
            connect(pingTimer_, &QTimer::timeout, this, &TcpClient::ping);
            pingTimer_->start();
        }
        if (reconnectSeconds_ > 0)
        {
            reconnectTimer_ = new QTimer(this);
            reconnectTimer_->setInterval(reconnectSeconds_ * 1000);
            connect(
                reconnectTimer_, &QTimer::timeout, this, &TcpClient::reconnect);
            reconnectTimer_->start();
        }
        connect(tcp_, &QTcpSocket::connected, this, &TcpClient::onConnected);
        connect(
            tcp_, &QTcpSocket::disconnected, this, &TcpClient::onDisconnected);
        connect(
            tcp_, &QAbstractSocket::errorOccurred, this, &TcpClient::onError);
        connect(tcp_, &QTcpSocket::readyRead, this, &TcpClient::onReadyRead);
        connectToServer();
        return true;
    }

    bool TcpClient::getConnect()
    {
        return bconnect_;
    }

    void TcpClient::connectToServer()
    {
        if (tcp_->state() == QAbstractSocket::UnconnectedState)
        {
            qDebug() << "connect to ip= " << host_ << ",port=" << port_;
            tcp_->connectToHost(host_, port_);
        }
        else
        {
            qDebug() << "connect state: " << tcp_->state()
                     << ",check QAbstractSocket::SocketState";
        }
    }

    bool TcpClient::sendData(const QByteArray &data)
    {
        bool ret = false;
        if (tcp_->state() == QAbstractSocket::ConnectedState)
        {
            tcp_->write(data);
            tcp_->flush();
            ret = true;
        }
        return ret;
    }

    void TcpClient::reconnect()
    {
        // 断开连接后重新连接到服务器
        qDebug() << "\nreconnect to server";
        connectToServer();
    }

    void TcpClient::onConnected()
    {
        bconnect_ = true;
        qDebug() << "\nConnected to ip, port: " << host_ << port_;
        // 连接成功后停止重连计时器
        if (reconnectTimer_ && reconnectTimer_->isActive())
            reconnectTimer_->stop();
    }

    void TcpClient::onDisconnected()
    {
        bconnect_ = false;
        // 断开连接后启动重连计时器
        if (reconnectTimer_ && !reconnectTimer_->isActive())
            reconnectTimer_->start();
    }

    void TcpClient::onError(QAbstractSocket::SocketError errCode)
    {
        bconnect_ = false;
        qDebug() << "\nSocket error: " << errCode
                 << ", check QAbstractSocket::SocketError";
        // 发生错误后启动重连计时器
        if (reconnectTimer_ && !reconnectTimer_->isActive())
            reconnectTimer_->start();
    }

    void TcpClient::onReadyRead()
    {
        QByteArray data = tcp_->readAll();
        emit recvData(data);
    }

    void TcpClient::ping()
    {
        sendData(heartContent_);
    }
} // namespace _Kits