#include "messagereceiver.h"

TcpMessageReceiver::TcpMessageReceiver(QTcpSocket *socket)
{
    current = new MessageHeader;
    this->socket = socket;
    connect(socket, SIGNAL(readyRead()), this, SLOT(readyRead()));
}

TcpMessageReceiver::~TcpMessageReceiver()
{
}

void TcpMessageReceiver::readyRead()
{
    int avail = socket->bytesAvailable();
    char *tmp = (char *)malloc(avail);
    int len = socket->read(tmp, avail);
    if (len < 0)
    {
        qDebug() << "TCP socket error occurs: " << socket->errorString();
        return;
    }

    buffer.append(QByteArray(tmp, len));
    processData();
}

#define DISPATCH_MESSAGE(signal, type) if (type *msg = dynamic_cast<type *>(current)) emit signal(*msg);

void TcpMessageReceiver::processData()
{
    while (current->length() <= buffer.size())
    {
        current->fill(buffer);
        switch (current->type())
        {
        case mtChat:                DISPATCH_MESSAGE(chatMessageReceived, ChatMessage); break;
        case mtClientConnect:       DISPATCH_MESSAGE(clientConnectMessageReceived, ClientConnectMessage); break;
        case mtClientDisconnect:    DISPATCH_MESSAGE(clientDisconnectMessageReceived, ClientDisconnectMessage); break;
        case mtConnectionAccepted:  DISPATCH_MESSAGE(connectionAcceptedMessageReceived, ConnectionAcceptedMessage); break;
        case mtPing:                DISPATCH_MESSAGE(pingMessageReceived, PingMessage); break;
        case mtPlayersList:         DISPATCH_MESSAGE(playersListMessageReceived, PlayersListMessage); break;
        case mtStartGame:           DISPATCH_MESSAGE(startGameMessageReceived, StartGameMessage); break;
        case mtServerReady:         DISPATCH_MESSAGE(serverReadyMessageReceived, ServerReadyMessage); break;
        case mtSurrender:           DISPATCH_MESSAGE(surrenderMessageReceived, SurrenderMessage); break;
        case mtTurn:                DISPATCH_MESSAGE(turnMessageReceived, TurnMessage); break;
        case mtTryToConnect:        DISPATCH_MESSAGE(tryToConnectMessageReceived, TryToConnectMessage); break;
        default: ;
        }

        buffer.remove(0, current->length());
        Message* old = current;
        current = current->next();
        delete old;
    }
}

UdpMessageReceiver::UdpMessageReceiver(QUdpSocket *socket)
{
    this->socket = socket;
    connect(socket, SIGNAL(readyRead()), this, SLOT(readyRead()));
}

UdpMessageReceiver::~UdpMessageReceiver()
{
}

void UdpMessageReceiver::readyRead()
{
    while (socket->hasPendingDatagrams())
    {
        QByteArray datagram;
        datagram.resize(socket->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;
        int len = socket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);
        if (len < 0)
        {
            qDebug() << "UDP socket error occurs: " << socket->errorString();
            return;
        }

        processData(datagram, sender, senderPort);
    }
}

void UdpMessageReceiver::processData(QByteArray &buffer, const QHostAddress &host, quint16 port)
{
    Message *current = new MessageHeader;
    while (current->length() <= buffer.size())
    {
        current->fill(buffer);
        switch (current->type())
        {
        case mtServerInfo:
            if (ServerInfoMessage *msg = dynamic_cast<ServerInfoMessage *>(current))
            {
                emit serverInfoMessageReceived(*msg, host, port);
            }
        case mtServerRequest:
            if (ServerRequestMessage *msg = dynamic_cast<ServerRequestMessage *>(current))
            {
                emit serverRequestMessageReceived(*msg, host, port);
            }
        default: ;
        }


        buffer.remove(0, current->length());
        Message* old = current;
        current = current->next();
        delete old;
    }
}
