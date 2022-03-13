#pragma once

#include "Common.h"

class Room;
class User;

class Lobby
{
public:
	Lobby(std::unordered_map<UINT64, User*>& users);
	~Lobby();

public:
	bool CreateRoom(User* user, std::string name, int maxCount);
	bool JoinRoom(User* user, std::string name);
	bool LeaveRoom(User* user, Room* room);
	void GetRoomList(User* user);

private:
	bool CanCreateRoom(User* user, std::string name);

	std::unordered_map<UINT64, User*>&     mUsers;
	std::unordered_map<std::string, Room*> mRooms;
};