#pragma once
#include <QObject>
#include <QTcpSocket>
#include <QTimer>

namespace _Kits
{
class TcpClient : public QObject
{
    Q_OBJECT

  public:
    TcpClient(const QString &host,
              quint16 port,
              int reconnectSeconds = 5,
              int heartSeconds = 0,
              QString heartContent = "",
              QObject *parent = nullptr);
    virtual ~TcpClient() noexcept;
    TcpClient(const TcpClient &) = delete;
    TcpClient &operator=(const TcpClient &) = delete;
    bool start();
    bool getConnect();
    bool sendData(const QByteArray &data);
  signals:
    void recvData(const QByteArray &data);

  private slots:
    void connectToServer();
    void onConnected();
    void onDisconnected();
    void onError(QAbstractSocket::SocketError errCode);
    void onReadyRead();
    void reconnect();
    void ping();

  private:
    QTcpSocket *tcp_ = nullptr;
    QTimer *reconnectTimer_ = nullptr;
    QTimer *pingTimer_ = nullptr;
    QString host_;
    QByteArray heartContent_;
    quint16 port_;
    quint16 reconnectSeconds_;
    quint16 heartSeconds_; // 心跳间隔，单位毫秒
    bool bconnect_ = false;
};
} // namespace _Kits
