#include "UdpClient.h"
namespace _Kits
{
UDPClient::UDPClient(QObject *parent)
    : QObject(parent), udp_(new QUdpSocket(this)), targetPort_(0)
{
    
}

UDPClient::~UDPClient()
{
}

bool UDPClient::bind(const QHostAddress &address, quint16 port)
{
    connect(udp_, &QUdpSocket::readyRead, this, &UDPClient::onReadyRead);
    return udp_->bind(address, port);

}
bool UDPClient::joinGroup(const QHostAddress &address)
{
        //udp_->setSocketOption()
       return udp_->joinMulticastGroup(address);
}
void UDPClient::setTarget(const QHostAddress &address, quint16 port)
{
    targetAddress_ = address;
    targetPort_ = port;
}

bool UDPClient::send(const QByteArray &data)
{
    if (targetAddress_.isNull() || targetPort_ == 0)
    {
        qWarning("Target address or port is not set.");
        return false;
    }

    qint64 bytesSent = udp_->writeDatagram(data, targetAddress_, targetPort_);
    return bytesSent == data.size();
}

void UDPClient::onReadyRead()
{
    while (udp_->hasPendingDatagrams())
    {
        QByteArray buffer;
        buffer.resize(udp_->pendingDatagramSize());

        QHostAddress sender;
        quint16 senderPort;

        udp_->readDatagram(buffer.data(), buffer.size(), &sender, &senderPort);
        emit dataReceived(buffer, sender, senderPort);
    }
}

void UDPClient::close()
{
    if (nullptr != udp_)
    {
        udp_->close();
    }
    
}
} // namespace _Kits