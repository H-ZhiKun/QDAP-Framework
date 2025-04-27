#pragma once
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>
#include <QVariantList>
#include <qjsonvalue.h>
#include <qobject.h>
#include <vector>

namespace _Kits
{
    class TcpServer : public QObject
    {
        Q_OBJECT

      public:
        explicit TcpServer(QObject *parent = nullptr);
        virtual ~TcpServer() noexcept;
        TcpServer(const TcpServer &) = delete;
        TcpServer &operator=(const TcpServer &) = delete;
        bool setPort(int port);
        bool startServer();
        bool stopServer();
      signals:
        void dataReceived(QTcpSocket *client, const QByteArray &data);
      public slots:
        bool sendData(QTcpSocket *client, const QByteArray &data);

      private slots:
        void onNewConnection();
        void onClientDisconnected();
        void onReadyRead();

      private:
        QString socketErrorToString(QAbstractSocket::SocketError socketError);

      private:
        QTcpServer *tcpServer = nullptr;
        std::vector<QTcpSocket *> clients;
        quint16 serverPort = 0;
        QByteArray m_buffer;
        quint32 m_expectedSize = 0;
    };
} // namespace _Kits
