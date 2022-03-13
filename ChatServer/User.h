#pragma once

#include "Common.h"

class Connection;
class Room;

class User
{
public:
	User(Connection* connection, std::string nickname);
	~User();

public:
	void   Send(char* message, int length);
	bool   IsEnterRoom();
	void   LeaveRoom();
	void   LeaveLobby();

	UINT64       GetId();
	Room*        GetRoom();
	void         SetRoom(Room* room);
	std::string& GetNickname();
	Connection*  GetConnection();

private:
	Connection* mConnection;
	std::string mNickname;
	Room*       mRoom;
};