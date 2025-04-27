#pragma once
#include <QHostAddress>
#include <QObject>
#include <QUdpSocket>
namespace _Kits
{

class UDPClient : public QObject
{
    Q_OBJECT
  public:
    explicit UDPClient(QObject *parent = nullptr);
    ~UDPClient();

    // 设置本地绑定地址和端口
    bool bind(const QHostAddress &address, quint16 port);
    // 添加组
    bool joinGroup(const QHostAddress &address);

    // 设置目标地址和端口
    void setTarget(const QHostAddress &address, quint16 port);

    // 发送数据
    bool send(const QByteArray &data);

    // 关闭套接字
    void close();

  signals:
    // 接收到数据时的信号
    void dataReceived(const QByteArray &data,
                      const QHostAddress &sender,
                      quint16 senderPort);

  private slots:
    // 处理数据接收
    void onReadyRead();

  private:
    QUdpSocket *udp_;
    QHostAddress targetAddress_;
    quint16 targetPort_;
};

} // namespace _Kits
