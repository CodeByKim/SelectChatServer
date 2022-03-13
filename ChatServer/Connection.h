#pragma once

#include "Common.h"
#include "RingBuffer.h"

class Connection
{
public:
	Connection(SOCKET sock, SOCKADDR_IN addr);
	~Connection();

public:
	void   RegisterForSelect(SelectSocketInfo* selectInfo, FD_SET* readSet, FD_SET* writeSet);
	void   Send(char* message, int length);
	void   SendBuffer();
	int    DoReceive(std::function<void(char* packet)> callback);
	bool   IsRecvData(FD_SET* readSet);
	bool   IsSendData(FD_SET* writeSet);

	SOCKET GetSocket();
	UINT64 GetId();

private:
	char*  MakePacket(int packetSize, int newlineSize);
	//char*  MakePacket(int packetSize);
	bool   HasSendData();

	SOCKET      mSocket;
	SOCKADDR_IN mAddr;
	RingBuffer  mRecvBuffer;
	RingBuffer  mSendBuffer;
	bool        mIsMakingPacket;
	bool        mDisconnect;
};