#include "User.h"
#include "Connection.h"
#include "Room.h"

User::User(Connection* connection, std::string nickname)
	: mConnection(connection)
	, mNickname(nickname)
	, mRoom(nullptr)
{
}

User::~User()
{
}

void User::Send(char* message, int length)
{
	mConnection->Send(message, length);
}

bool User::IsEnterRoom()
{
	return mRoom != nullptr;
}

void User::LeaveRoom()
{
	mRoom->Leave(this);
}

void User::LeaveLobby()
{
}

UINT64 User::GetId()
{
	return mConnection->GetId();
}

Room* User::GetRoom()
{
	return mRoom;
}

void User::SetRoom(Room* room)
{
	mRoom = room;
}

std::string& User::GetNickname()
{	
	return mNickname;
}

Connection* User::GetConnection()
{
	return mConnection;
}
