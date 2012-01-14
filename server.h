#ifndef _SERVER_H_
#define _SERVER_H_

#include "client.h"
#include "message.h"
#include <QTimer>
#include <QtNetwork/QTcpServer>
#include <QtNetwork/QUdpSocket>

class Server : public QThread {
    Q_OBJECT
private:
    QTcpServer serverConnection;
    QUdpSocket listener;
    MessageReceiver *messageReceiver;
    QList<RemoteClient*> clients;
    QTimer timer;
    int maxClientsCount;
public:
    int getMaxClientsCount() {return maxClientsCount;}
    Server();
    ~Server();
    QString getErrorString() const {return serverConnection.errorString();}
    int getPlayersCount() const {int count = 0;for(int i=0;i<clients.size();++i) if (clients[i]->state==2/*&&clients[i]->socket->isConnected()*/) ++count;return count;}
    bool start(int maxClientsCount, quint16 port);
    QList<ClientInfo> getClients() const;
private:
    void stop();
    void sendPlayersList();
    void sendToAll(Message*);
    void removeClient(RemoteClient*);
protected:
    void run();
private slots:
    //from app
    void startGame();
    void restartGame(QList<ClientInfo>);
    //from timer
    void ping();
    //from message receiver
    void serverRequestMessageReceive(ServerRequestMessage, const QHostAddress &, quint16);
    //from tcpserver
    void newConnection();
    //from remote client
    void remoteChatMessageReceive(ChatMessage,RemoteClient*);
    void remoteTryToConnectMessageReceive(TryToConnectMessage,RemoteClient*);
    void remotePingMessageReceive(PingMessage,RemoteClient*);
    void remoteTurnMessageReceive(TurnMessage,RemoteClient*);
    void remoteSurrenderMessageReceive(SurrenderMessage,RemoteClient*);
    void remoteDisconnected(RemoteClient*);
    void remoteError(RemoteClient*);
};

#endif
