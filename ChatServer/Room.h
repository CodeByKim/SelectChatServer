#pragma once

#include "Common.h"

class User;

class Room
{
public:
	Room(std::string name, int maxCount);
	~Room();

public:
	bool         Enter(User* user);
	bool         Leave(User* user);
	void         Broadcast(char* message, int length);
	void         Broadcast(char* message, int length, User* exceptUser);
	std::string& GetName();
	int          GetUserCount();

private:
	std::string      mName;
	int              mMaxCount;
	std::list<User*> mUsers;
};