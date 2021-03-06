#include "localclient.h"
#include "constants.h"

LocalClient::LocalClient() : _isStarted(false), _lastPingTime(QTime::currentTime())
{
    _socket = new QTcpSocket(this);
    _messageReceiver = new TcpMessageReceiver(_socket, this);
    connect(_messageReceiver, SIGNAL(chatMessageReceived(ChatMessage)), this, SLOT(receiveChatMessage(ChatMessage)));
    connect(_messageReceiver, SIGNAL(clientConnectMessageReceived(ClientConnectMessage)), this, SLOT(receiveClientConnectMessage(ClientConnectMessage)));
    connect(_messageReceiver, SIGNAL(clientDisconnectMessageReceived(ClientDisconnectMessage)), this, SLOT(receiveClientDisconnectMessage(ClientDisconnectMessage)));
    connect(_messageReceiver, SIGNAL(connectionAcceptedMessageReceived(ConnectionAcceptedMessage)), this, SLOT(receiveConnectionAcceptedMessage(ConnectionAcceptedMessage)));
    connect(_messageReceiver, SIGNAL(errorMessageReceived(ErrorMessage)), this, SLOT(receiveErrorMessage(ErrorMessage)));
    connect(_messageReceiver, SIGNAL(pingMessageReceived(PingMessage)), this, SLOT(receivePingMessage(PingMessage)));
    connect(_messageReceiver, SIGNAL(playersListMessageReceived(PlayersListMessage)), this, SLOT(receivePlayersListMessage(PlayersListMessage)));
    connect(_messageReceiver, SIGNAL(serverReadyMessageReceived(ServerReadyMessage)), this, SLOT(receiveServerReadyMessage(ServerReadyMessage)));
    connect(_messageReceiver, SIGNAL(startGameMessageReceived(StartGameMessage)), this, SLOT(receiveStartGameMessage(StartGameMessage)));
    connect(_messageReceiver, SIGNAL(surrenderMessageReceived(SurrenderMessage)), this, SLOT(receiveSurrenderMessage(SurrenderMessage)));
    connect(_messageReceiver, SIGNAL(turnMessageReceived(TurnMessage)), this, SLOT(receiveTurnMessage(TurnMessage)));
    connect(_socket, SIGNAL(disconnected()), this, SLOT(processSocketDisconnected()));
    connect(_socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(processSocketError(QAbstractSocket::SocketError)));

    connect(this, SIGNAL(destroyed()), this, SLOT(stop()));

    _timer = new QTimer(this);
    _timer->setInterval(PING_INTERVAL);
    connect(_timer, SIGNAL(timeout()), this, SLOT(timeout()));
}

void LocalClient::start(const QString &hostname, quint16 port)
{
    if (!_isStarted)
    {
        _isStarted = true;
        _socket->connectToHost(hostname, port);
        _timer->start();
        _lastPingTime.start();
    }
}

void LocalClient::stop()
{
    if (_isStarted)
    {
        _isStarted = false;
        _timer->stop();
        _socket->disconnectFromHost();
    }
}

void LocalClient::receiveChatMessage(const ChatMessage &msg)
{
    emit chatMessageReceived(msg.info(), msg.text());
}

void LocalClient::receiveClientConnectMessage(const ClientConnectMessage &msg)
{
    emit clientConnectMessageReceived(msg.info());
}

void LocalClient::receiveClientDisconnectMessage(const ClientDisconnectMessage &msg)
{
    emit clientDisconnectMessageReceived(msg.info());
}

void LocalClient::receiveConnectionAcceptedMessage(const ConnectionAcceptedMessage &)
{
    emit connectionAccepted();
}

void LocalClient::receiveErrorMessage(const ErrorMessage &msg)
{
    switch (msg.errorCode())
    {
    case ERROR_COLOR_IN_USE:
    case ERROR_GAME_STARTED:
    case ERROR_NAME_IN_USE:
    case ERROR_MAX_CONNECTIONS_NUM:
    case ERROR_MAX_PLAYERS_NUM:
        stop();
    }

    emit errorOccurred(msg.errorCode());
}

void LocalClient::receivePingMessage(const PingMessage &msg)
{
    msg.send(_socket);
    _lastPingTime.start();
}

void LocalClient::receivePlayersListMessage(const PlayersListMessage &msg)
{
    emit playersListMessageReceived(msg.list());
}

void LocalClient::receiveStartGameMessage(const StartGameMessage &msg)
{
    emit startGameMessageReceived(msg.list());
}

void LocalClient::receiveServerReadyMessage(const ServerReadyMessage &)
{
    TryToConnectMessage msg(_info);
    msg.send(_socket);
}

void LocalClient::receiveSurrenderMessage(const SurrenderMessage &msg)
{
    emit surrenderMessageReceived(msg.info());
}

void LocalClient::receiveTurnMessage(const TurnMessage &msg)
{
    emit turnMessageReceived(msg.color(), msg.mask(), msg.id(), msg.x(), msg.y());
}

void LocalClient::processSocketDisconnected()
{
    if (_isStarted)
    {
        _isStarted = false;
        _timer->stop();
    }

    emit disconnected();
}

void LocalClient::processSocketError(QAbstractSocket::SocketError)
{
    stop();
    emit errorOccurred(ERROR_SOCKET_ERROR);
}

void LocalClient::sendChatMessage(const ClientInfo &info, const QString &text)
{
    ChatMessage msg(info, text);
    msg.send(_socket);
}

void LocalClient::sendStartGameMessage()
{
    ServerStartGameMessage msg;
    msg.send(_socket);
}

void LocalClient::sendStopGameMessage()
{
    ServerStopGameMessage msg;
    msg.send(_socket);
}

void LocalClient::sendSurrenderMessage(const ClientInfo &info)
{
    SurrenderMessage msg(info);
    msg.send(_socket);
}

void LocalClient::sendTurnMessage(const ClientInfo &info, const QString &tile, int id, int x, int y)
{
    TurnMessage msg(info, tile, id, x, y);
    msg.send(_socket);
}

void LocalClient::timeout()
{
    int elapsed = _lastPingTime.elapsed();
    if (elapsed > PING_TIME)
    {
        stop();
        emit errorOccurred(ERROR_PING_TIMEOUT);
    }
}
