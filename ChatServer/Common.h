#pragma once
#pragma comment (lib, "ws2_32")

#include <WinSock2.h>
#include <Windows.h>

#include <iostream>
#include <vector>
#include <queue>
#include <stack>
#include <unordered_map>
#include <memory>
#include <string>
#include <string_view>
#include <sstream>
#include <functional>
#include <algorithm>

#define BUFFER_SIZE 256
#define SERVER_PORT 3000

struct SelectSocketInfo
{
	SOCKET sockets[FD_SETSIZE];
	int    count;

	SelectSocketInfo()
		: sockets{}
		, count(0)
	{
	}

	void Reset()
	{
		ZeroMemory(sockets, sizeof(sockets));
		count = 0;
	}
};