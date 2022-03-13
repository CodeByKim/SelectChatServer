#include "Lobby.h"
#include "Room.h"
#include "User.h"

Lobby::Lobby(std::unordered_map<UINT64, User*>& users)
	: mUsers(users)
{
}

Lobby::~Lobby()
{
}

bool Lobby::CreateRoom(User* user, std::string name, int maxCount)
{
	if (!CanCreateRoom(user, name))
		return false;
	
	Room* room = new Room(name, maxCount);
	room->Enter(user);
	mRooms.insert(std::make_pair(name, room));

	return true;
}

bool Lobby::JoinRoom(User* user, std::string name)
{
	auto iter = mRooms.find(name);
	if (iter == mRooms.end())
		return false;

	Room* room = iter->second;
	if (!room->Enter(user))
		return false;

	return true;
}

bool Lobby::LeaveRoom(User* user, Room* room)
{	
	if (!room->Leave(user))
		return false;

	if (room->GetUserCount() == 0)
	{
		mRooms.erase(room->GetName());
		delete room;
	}

	return true;
}

void Lobby::GetRoomList(User* user)
{
	std::string rooms;
	for (auto& [name, room] : mRooms)
	{
		rooms += room->GetName();
		rooms += "\r\n";
	}

	char buffer[256];
	sprintf_s(buffer, rooms.c_str());
	user->Send(buffer, (int)strlen(buffer));
}

bool Lobby::CanCreateRoom(User* user, std::string name)
{
	if (user->IsEnterRoom())
	{
		char message[] =
			"로비에서 하세요.\r\n"
			"선택> ";

		user->Send(message, sizeof(message));
		return false;
	}

	if (mRooms.find(name) != mRooms.end())
		return false;

	return true;
}
