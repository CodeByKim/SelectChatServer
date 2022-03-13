#include "Room.h"
#include "User.h"

Room::Room(std::string name, int maxCount)
	: mName(name)
	, mMaxCount(maxCount)
{
}

Room::~Room()
{
	mUsers.clear();
}

bool Room::Enter(User* user)
{
	if (mMaxCount < mUsers.size() + 1)
	{
		char message[] =
			"사용자 수 초과.\r\n";

		user->Send(message, sizeof(message));
		return false;
	}

	mUsers.push_back(user);
	user->SetRoom(this);

	char message[] =
		"** %s님이 입장하셨습니다. (현재인원 %d/%d)\r\n"
		"선택> ";

	char buffer[256];
	sprintf_s(buffer, message, user->GetNickname().c_str(), GetUserCount(), mMaxCount);
	Broadcast(buffer, (int)strlen(buffer));

	return true;
}

bool Room::Leave(User* user)
{
	auto iter = mUsers.begin();
	for (; iter != mUsers.end(); ++iter)
	{
		if (*iter == user)
		{
			mUsers.erase(iter);
			break;
		}
	}

	char message[] = "%s님이 퇴장하셨습니다. (현재인원 %d/%d)\r\n";
	char buffer[256];
	sprintf_s(buffer, message, user->GetNickname().c_str(), GetUserCount(), mMaxCount);
	Broadcast(buffer, (int)strlen(buffer), user);

	user->SetRoom(nullptr);
	return true;
}

void Room::Broadcast(char* message, int length)
{
	for (auto iter : mUsers)
		iter->Send(message, length);
}

void Room::Broadcast(char* message, int length, User* exceptUser)
{
	for (auto iter : mUsers)
	{
		if (iter == exceptUser)
			continue;

		iter->Send(message, length);
	}
}

std::string& Room::GetName()
{
	return mName;
}

int Room::GetUserCount()
{
	return (int)mUsers.size();
}
