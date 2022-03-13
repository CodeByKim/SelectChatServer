#include "Acceptor.h"
#include "Connection.h"

Acceptor::Acceptor()
	: mSocket(INVALID_SOCKET)	
	, mAddr {}
{	
}

Acceptor::~Acceptor()
{
}

void Acceptor::Listen(int backlog)
{
	mSocket = socket(PF_INET, SOCK_STREAM, 0);

	ZeroMemory(&mAddr, sizeof(SOCKADDR_IN));
	mAddr.sin_family = AF_INET;
	mAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	mAddr.sin_port = htons(SERVER_PORT);

	bind(mSocket, (SOCKADDR*)&mAddr, sizeof(mAddr));
	listen(mSocket, backlog);

	u_long on = 1;
	ioctlsocket(mSocket, FIONBIO, &on);
}

void Acceptor::RegisterForSelect(SelectSocketInfo* selectInfo, FD_SET* readSet)
{
	FD_SET(mSocket, readSet);

	selectInfo->sockets[0] = mSocket;
	selectInfo->count += 1;
}

Connection* Acceptor::Accept(FD_SET* readSet)
{
	if (FD_ISSET(mSocket, readSet))
	{
		SOCKADDR_IN clientAddr;
		int clientAddrSize = sizeof(clientAddr);
		SOCKET sock = accept(mSocket, (SOCKADDR*)&clientAddr, &clientAddrSize);
		Connection* conn = new Connection(sock, clientAddr);

		return conn;
	}

	return nullptr;
}

SOCKET Acceptor::GetSocket()
{
	return mSocket;
}