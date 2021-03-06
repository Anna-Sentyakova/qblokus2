#include "message.h"

void Message::send(QTcpSocket *socket) const
{
    QByteArray data = serialize();
    socket->write(data.data(), data.size());
}

void Message::send(QUdpSocket *socket, const QHostAddress &host, quint16 port) const
{
    QByteArray data = serialize();
    socket->writeDatagram(data, host, port);
}

QByteArray MessageHeader::serialize() const
{
    QByteArray result;
    result.append(QByteArray::fromRawData((const char *)&_msgType, sizeof(MessageType)));
    result.append(QByteArray::fromRawData((const char *)&_msgLen, sizeof(qint64)));
    return result;
}

void MessageHeader::fill(const QByteArray &buffer)
{
    const char *data = buffer.data();
    ::memmove(&_msgType, data, sizeof(MessageType));
    data += sizeof(MessageType);
    ::memmove(&_msgLen, data, sizeof(qint64));
}

ChatMessage::ChatMessage(const ClientInfo &info, const QString &text) {
    _info.setName(info.name());
    _info.setColor(info.color());
    _text = text;
    _header.setMsgLength(_info.size() + _text.toUtf8().size() + sizeof(int));
    _header.setMsgType(mtChat);
}

QByteArray ChatMessage::serialize() const
{
    QByteArray result;
    result.append(_info.serialize());
    QByteArray tmp = _text.toUtf8();
    int size = tmp.size();
    result.append(QByteArray::fromRawData((const char *)&size, sizeof(int)));
    result.append(tmp);
    return _header.serialize().append(result);
}

void ChatMessage::fill(const QByteArray &buffer)
{
    const char *data = buffer.data();
    _info.fill(data);
    data += _info.size();
    int textlen = *((int *)data);
    data += sizeof(int);
    _text = QString::fromUtf8(data, textlen);
}

QByteArray ClientMessage::serialize() const
{
    QByteArray result;
    result.append(_info.serialize());
    return _header.serialize().append(result);
}

void ClientMessage::fill(const QByteArray &buffer)
{
    const char *data = buffer.data();
    _info.fill(data);
}

ClientConnectMessage::ClientConnectMessage(const ClientInfo &info)
{
    _info = info;
    _header.setMsgLength(_info.size());
    _header.setMsgType(mtClientConnect);
}

ClientDisconnectMessage::ClientDisconnectMessage(const ClientInfo &info)
{
    _info = info;
    _header.setMsgLength(_info.size());
    _header.setMsgType(mtClientDisconnect);
}

ErrorMessage::ErrorMessage(int errorCode)
{
    _errorCode = errorCode;
    _header.setMsgLength(sizeof(int));
    _header.setMsgType(mtError);
}

QByteArray ErrorMessage::serialize() const
{
    QByteArray result = QByteArray::fromRawData((const char *)(&_errorCode), sizeof(int));
    return _header.serialize().append(result);
}

void ErrorMessage::fill(const QByteArray &buffer)
{
    const char *data = buffer.data();
    ::memmove((void *)(&_errorCode), data, sizeof(int));
}

PlayersListMessage::PlayersListMessage(QList<ClientInfo> list)
{
    _list = list;
    qint64 len = sizeof(int);
    for (int i = 0; i < list.size(); len += list[i++].size()) { }
    _header.setMsgLength(len);
    _header.setMsgType(mtPlayersList);
}

QByteArray PlayersListMessage::serialize() const
{
    QByteArray result;
    int size = _list.size();
    result.append(QByteArray::fromRawData((const char *)&size, sizeof(int)));
    for (int i = 0; i < _list.size(); result.append(_list[i++].serialize())) { }
    return _header.serialize().append(result);
}

void PlayersListMessage::fill(const QByteArray &buffer)
{
    const char *data = buffer.data();
    int count = *((int *)data);
    data += sizeof(int);
    for (int i = 0; i < count; ++i)
    {
        ClientInfo item;
        item.fill(data);
        data += item.size();
        _list.append(item);
    }
}

ServerInfoMessage::ServerInfoMessage(QList<ClientInfo> list)
{
    _list = list;
    qint64 len = sizeof(int);
    for (int i = 0; i < list.size(); len += list[i++].size()) { }
    _header.setMsgLength(len);
    _header.setMsgType(mtServerInfo);
}

StartGameMessage::StartGameMessage(QList<ClientInfo> list)
{
    _list = list;
    qint64 len = sizeof(int);
    for (int i = 0; i < list.size(); len += list[i++].size()) { }
    _header.setMsgLength(len);
    _header.setMsgType(mtStartGame);
}

SurrenderMessage::SurrenderMessage(const ClientInfo &info)
{
    _info.setName(info.name());
    _info.setColor(info.color());
    _header.setMsgLength(_info.size());
    _header.setMsgType(mtSurrender);
}

TryToConnectMessage::TryToConnectMessage(const ClientInfo &info)
{
    _info = info;
    _header.setMsgLength(_info.size());
    _header.setMsgType(mtTryToConnect);
}

TurnMessage::TurnMessage(const ClientInfo &info, const QString &tile, int id, int x, int y)
{
    _info.setName(info.name());
    _info.setColor(info.color());
    _id = id;
    _x = x;
    _y = y;
    _mask = tile;
    _header.setMsgLength(_info.size() + 4 * sizeof(int) + _mask.toUtf8().size());
    _header.setMsgType(mtTurn);
}

QByteArray TurnMessage::serialize() const
{
    QByteArray result = _info.serialize();
    result.append(QByteArray::fromRawData((const char *)&_id, sizeof(int)));
    result.append(QByteArray::fromRawData((const char *)&_x, sizeof(int)));
    result.append(QByteArray::fromRawData((const char *)&_y, sizeof(int)));
    QByteArray tmp = _mask.toUtf8();
    int size = tmp.size();
    result.append(QByteArray::fromRawData((const char *)&size, sizeof(int)));
    result.append(tmp);
    return _header.serialize().append(result);
}

void TurnMessage::fill(const QByteArray &buffer)
{
    const char *data = buffer.data();
    _info.fill(data);
    data += _info.size();
    _id = *((int *)data);
    data += sizeof(int);
    _x = *((int *)data);
    data += sizeof(int);
    _y = *((int *)data);
    data += sizeof(int);
    int textlen = *((int *)data);
    data += sizeof(int);
    _mask = QString::fromUtf8(data, textlen);
}
