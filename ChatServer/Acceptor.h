#pragma once

#include "Common.h"

class Connection;

class Acceptor
{
public:
	Acceptor();
	~Acceptor();

public:
	void        Listen(int backlog);
	void        RegisterForSelect(SelectSocketInfo* selectInfo, FD_SET* readSet);
	Connection* Accept(FD_SET* readSet);
	SOCKET      GetSocket();

private:
	SOCKET      mSocket;
	SOCKADDR_IN mAddr;	
};