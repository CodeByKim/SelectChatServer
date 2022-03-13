#pragma once

#include "Common.h"
#include "Acceptor.h"

class Connection;

enum class eEventType
{
	Connect,
	Receive,
	Send,
	Disconnect
};

struct ConnectionEvent
{
	Connection* connection;
	eEventType  eventType;
	int         tranferBytes;
};

class Server
{
public:
	Server();
	~Server();

public:
	void         Run();
	int          GetConnectedConnectionCount();
	void         CloseConnection(Connection* connection);

	virtual void OnAccept(ConnectionEvent& connectionEvent) = 0;
	virtual void OnDisconnect(ConnectionEvent& connectionEvent) = 0;
	virtual void OnReceive(ConnectionEvent& connectionEvent, char* packet) = 0;

private:
	void         ConnectNewConnection(Connection* connection);
	void         ReserveDisconnect(Connection* connection);
	bool         PollNetworkEvent();
	void         ProcessDisconnectedConnection();
	void         SelectSocket(SelectSocketInfo* selectInfo);
	void         PrintPacket(char* packet);
	bool         CheckSelectSocket(int result);

	Acceptor                                mAcceptor;
	FD_SET                                  mReadSet;
	FD_SET                                  mWriteSet;
	std::unordered_map<SOCKET, Connection*> mConnections;
	std::vector<Connection*>                mReserveDisconnectConnections;
};