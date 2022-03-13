#include "Connection.h"

Connection::Connection(SOCKET sock, SOCKADDR_IN addr)
	: mSocket(sock)
	, mAddr(addr)
	, mRecvBuffer(1024)
	, mSendBuffer(1024)
	, mIsMakingPacket(false)
	, mDisconnect(false)
{	
	u_long on = 1;
	ioctlsocket(mSocket, FIONBIO, &on);
}

Connection::~Connection()
{
	closesocket(mSocket);
}

void Connection::RegisterForSelect(SelectSocketInfo* selectInfo, FD_SET* readSet, FD_SET* writeSet)
{
	FD_SET(mSocket, readSet);

	selectInfo->sockets[selectInfo->count] = mSocket;
	selectInfo->count += 1;

	if (HasSendData())
		FD_SET(mSocket, writeSet);
}

void Connection::Send(char* message, int length)
{
	if (!mSendBuffer.Enqueue(message, length))
	{
		std::cout << "send buffer is full" << std::endl;
		mDisconnect = true;
	}
}

void Connection::SendBuffer()
{
	int useSize = mSendBuffer.GetUseSize();
	if (useSize <= 0)
		return;

	int sendBytes = send(mSocket,
		                 mSendBuffer.GetBufferFront(),
		                 mSendBuffer.GetDirectDequeueSize(),
		                 0);

	if (sendBytes == SOCKET_ERROR)
	{
		int error = WSAGetLastError();
		if (error == WSAEWOULDBLOCK)
			return;
		else
			std::cout << "send error : " << error << std::endl;
	}
	else
	{
		mSendBuffer.MoveFront(sendBytes);
	}
}

int Connection::DoReceive(std::function<void(char* packet)> callback)
{
	if (mDisconnect)
		return SOCKET_ERROR;

	char buffer[BUFFER_SIZE];
	int recvBytes = recv(mSocket, buffer, BUFFER_SIZE, 0);
	if (!mRecvBuffer.Enqueue(buffer, recvBytes))
	{
		std::cout << "recv buffer is full" << std::endl;
		mDisconnect = true;

		return SOCKET_ERROR;
	}
	
	for (int i = 0; i < recvBytes; i++)
	{
		if (buffer[i] == '\r')
		{
			mIsMakingPacket = true;
			continue;
		}

		if (buffer[i] == '\n')
		{
			char* packet = nullptr;
			int newlineSize = 0;
			int packetSize = 0;

			if (mIsMakingPacket)
			{
				// 2바이트 개행 처리
				// packetSize는 \r\n 포함 길이 ex)[a][b][c][\r][\n], 5바이트
				newlineSize = 2;
				packetSize = mRecvBuffer.GetUseSize() - (recvBytes - i - 1);
			}
			else
			{
				// 1바이트 개행 처리
				// packetSize는 \n 포함 길이 ex)[a][b][c][\n], 4바이트
				newlineSize = 1;
				packetSize = mRecvBuffer.GetUseSize() - (recvBytes - i);
			}

			packet = MakePacket(packetSize - newlineSize, newlineSize);
			callback(packet);
			delete[] packet;

			mIsMakingPacket = false;
		}
	}

	return recvBytes;
}

bool Connection::IsRecvData(FD_SET* readSet)
{
	if (FD_ISSET(mSocket, readSet) || mDisconnect)
		return true;

	return false;
}

bool Connection::IsSendData(FD_SET* writeSet)
{
	if (FD_ISSET(mSocket, writeSet))
		return true;

	return false;
}

SOCKET Connection::GetSocket()
{
	return mSocket;	
}

UINT64 Connection::GetId()
{
	return (UINT64)mSocket;
}

char* Connection::MakePacket(int packetSize, int newlineSize)
{
	char* packet = new char[packetSize + 1];
	mRecvBuffer.Dequeue(packet, packetSize);
	mRecvBuffer.MoveFront(newlineSize);
	packet[packetSize] = '\0';

	return packet;
}

bool Connection::HasSendData()
{
	return mSendBuffer.GetUseSize() > 0;
}