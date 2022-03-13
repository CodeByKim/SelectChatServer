#include "Server.h"
#include "Connection.h"

Server::Server()
{
	WSAData data;
	if (WSAStartup(MAKEWORD(2, 2), &data) != 0)
	{
		std::cout << "initialize error" << std::endl;
		return;
	}

	mAcceptor.Listen(SOMAXCONN);
	FD_ZERO(&mReadSet);
	FD_ZERO(&mWriteSet);
}

Server::~Server()
{
	WSACleanup();
}

void Server::Run()
{
	while (true)
	{
		if (!PollNetworkEvent())
			return;

		ProcessDisconnectedConnection();
	}
}

int Server::GetConnectedConnectionCount()
{
	return (int)mConnections.size();
}

void Server::CloseConnection(Connection* connection)
{
	auto iter = mConnections.find(connection->GetSocket());
	if (iter != mConnections.end())
	{
		ReserveDisconnect(connection);
	}
	else
	{
		if (connection != nullptr)
			delete connection;
	}
}

void Server::ConnectNewConnection(Connection* connection)
{
	SOCKET sock = connection->GetSocket();
	mConnections.insert(std::make_pair(sock, connection));

	ConnectionEvent connEvent = { connection, eEventType::Connect, 0 };
	OnAccept(connEvent);
}

void Server::ReserveDisconnect(Connection* connection)
{		
	mReserveDisconnectConnections.push_back(connection);	
}

bool Server::PollNetworkEvent()
{	
	FD_ZERO(&mReadSet);
	FD_ZERO(&mWriteSet);
	
	SelectSocketInfo selectInfo;
	mAcceptor.RegisterForSelect(&selectInfo, &mReadSet);

	for (auto& [socketHandle, connection] : mConnections)
	{		
		connection->RegisterForSelect(&selectInfo , &mReadSet, &mWriteSet);

		if (FD_SETSIZE <= selectInfo.count)
		{
			SelectSocket(&selectInfo);

			FD_ZERO(&mReadSet);
			FD_ZERO(&mWriteSet);
			selectInfo.Reset();
		}
	}

	if (selectInfo.count > 0)
	{
		SelectSocket(&selectInfo);
	}

	return true;
}

void Server::ProcessDisconnectedConnection()
{
	for (auto conn : mReserveDisconnectConnections)
	{
		ConnectionEvent connEvent = { conn, eEventType::Disconnect, 0 };
		OnDisconnect(connEvent);

		SOCKET sock = conn->GetSocket();
		delete conn;

		mConnections.erase(sock);
	}

	mReserveDisconnectConnections.clear();
}

void Server::SelectSocket(SelectSocketInfo* selectInfo)
{
	TIMEVAL time;
	time.tv_sec = 0;
	time.tv_usec = 0;

	int result = select(0, &mReadSet, &mWriteSet, 0, &time);
	if (!CheckSelectSocket(result))
		return;

	for (int i = 0; i < selectInfo->count; i++)
	{
		if (i == 0)
		{
			Connection* newConnection = mAcceptor.Accept(&mReadSet);
			if (newConnection != nullptr)
				ConnectNewConnection(newConnection);

			continue;
		}

		SOCKET sock = selectInfo->sockets[i];
		Connection* connection = mConnections[sock];

		if(connection->IsRecvData(&mReadSet))
		{
			int recvBytes = connection->DoReceive(
				[this, connection, recvBytes](char* packet)
				{
					PrintPacket(packet);

					ConnectionEvent connEvent = { connection, eEventType::Receive, recvBytes };
					OnReceive(connEvent, packet);
				});

			if (recvBytes == 0 || recvBytes == SOCKET_ERROR)
			{
				ReserveDisconnect(connection);
				continue;
			}
		}

		if (connection->IsSendData(&mWriteSet))
		{			
			Connection* connection = mConnections[sock];
			connection->SendBuffer();
		}
	}
}

void Server::PrintPacket(char* packet)
{
	std::cout << packet << std::endl;
}

bool Server::CheckSelectSocket(int result)
{
	if (result == SOCKET_ERROR)
	{
		std::cout << "select error : " << WSAGetLastError() << std::endl;
		return false;
	}

	if (result <= 0)
		return false;

	return true;
}
