#include "ChatServer.h"
#include "Connection.h"
#include "RingBuffer.h"
#include "Util.h"
#include "User.h"
#include "Room.h"

ChatServer::ChatServer()
	: mLobby(mUsers)
{
}

ChatServer::~ChatServer()
{
}

void ChatServer::OnAccept(ConnectionEvent& connectionEvent)
{
	if (connectionEvent.eventType != eEventType::Connect)
		return;

	Connection* connection = connectionEvent.connection;
	std::cout << "new connection : " << connection->GetId() << std::endl;

	char message[] =
		"** �ȳ��ϼ��� �ؽ�Ʈ ä�ü��� �Դϴ�.\r\n"
		"** LOGIN ��ɾ ����ϼ���.\r\n"
		"����> ";

	connection->Send(message, sizeof(message));
}

void ChatServer::OnReceive(ConnectionEvent& connectionEvent, char* packet)
{
	if (connectionEvent.eventType != eEventType::Receive)
		return;

	ParsePacket(connectionEvent.connection, packet);
}

void ChatServer::OnDisconnect(ConnectionEvent& connectionEvent)
{
	if (connectionEvent.eventType != eEventType::Disconnect)
		return;

	UINT64 id = connectionEvent.connection->GetId();
	std::cout << "close connection : " << id << std::endl;

	auto iter = mUsers.find(id);
	if (iter != mUsers.end())
	{
		delete iter->second;
		mUsers.erase(id);
	}
}

int ChatServer::GetCommand(User* user, std::string& command)
{
	std::string result;
	bool isSlashCommand = false;

	if (command[0] == '/')
	{		
		ToUpperString(&result,
			          command.begin() + 1, 
			          command.end(), 
			          (int)command.size() - 1);

		isSlashCommand = true;
	}
	else
	{
		ToUpperString(&result,
			          command.begin(),
			          command.end(),
			          (int)command.size());
	}
	
	if (user == nullptr)
	{
		if (isSlashCommand)
			return COMMAND_ERROR;

		return result == "LOGIN" ? COMMAND_LOGIN : COMMAND_ERROR;
	}
	else
	{
		if (user->IsEnterRoom())
		{			
			if (!isSlashCommand)
				return COMMAND_CHAT;
		}
		else
		{
			if (isSlashCommand)
				return COMMAND_ERROR;
		}
	}

	if (result == "LOGIN")
		return COMMAND_LOGIN;
	else if (result == "US")
		return COMMAND_GET_USER_LIST;
	else if (result == "LT")
		return COMMAND_GET_ROOM_LIST;
	else if (result == "O")
		return COMMAND_CREATE_ROOM;
	else if (result == "J")
		return COMMAND_JOIN_ROOM;
	else if (result == "TO")
		return COMMAND_WHISPER_CHAT;
	else if (result == "H")
		return COMMAND_GET_COMMAND_LIST;
	else if (result == "X")
		return COMMAND_LEAVE;
	else
		return COMMAND_ERROR;
}

void ChatServer::ParsePacket(Connection* connection, char* packet)
{
	if (connection == nullptr)
	{
		CloseConnection(connection);
		return;
	}

	User* user = nullptr;
	if (IsConnectUser(connection->GetId()))
		user = mUsers[connection->GetId()];
	
	std::string str(packet);
	std::vector<std::string> splits;

	SplitString(&str, &splits);
	int command = GetCommand(user, splits[0]);

	if (!user)
	{
		if (command == COMMAND_LOGIN)
			LoginHandler(connection, &splits);
		else
		{
			char message[] = "�α��κ��� �ϼ���.\r\n";
			connection->Send(message, sizeof(message));
		}
		return;
	}

	switch(command)
	{	
	case COMMAND_GET_USER_LIST:
		GetUserListHandler(user, &splits);
		break;
	case COMMAND_GET_ROOM_LIST:
		GetRoomListHandler(user, &splits);
		break;
	case COMMAND_CREATE_ROOM:
		CreateRoomHandler(user, &splits);
		break;
	case COMMAND_JOIN_ROOM:
		JoinRoomHandler(user, &splits);
		break;
	case COMMAND_WHISPER_CHAT:
		WhisperChatHandler(user, &splits);
		break;
	case COMMAND_GET_COMMAND_LIST:
		GetCommandListHandler(user, &splits);
		break;
	case COMMAND_CHAT:		
		ChatHandler(user, packet, (int)strlen(packet));
		break;
	case COMMAND_LEAVE:
		LeaveHandler(user, &splits);
		break;
	case COMMAND_ERROR:
		SendRetryMessage(user);
		break;
	}
}

void ChatServer::SendRetryMessage(Connection* connection)
{
	char message[] =
		"�ٽ� �Է��ϼ���.\r\n"
		"����> ";

	connection->Send(message, sizeof(message));
}

void ChatServer::SendRetryMessage(User* user)
{
	char message[] =
		"�ٽ� �Է��ϼ���.\r\n"
		"����> ";

	user->Send(message, sizeof(message));
}

void ChatServer::SendShowCommandList(User* user)
{
	char message[] =
		"--------------------------------------\r\n"
		"H                      ��ɾ� �ȳ�\r\n"
		"US                     �̿��� ��� ����\r\n"
		"LT                     ��ȭ�� ��� ����\r\n"
		"TO [���̵�]   [�޽���] ���� ������\r\n"
		"O  [�ִ��ο�] [������] ��ȭ�� �����\r\n"
		"J  [������]            ��ȭ�� �����ϱ�\r\n"
		"--------------------------------------\r\n"
		"��ɾ�ȳ�(H) ����(X)\r\n"
		"����> ";

	user->Send(message, sizeof(message));
}

bool ChatServer::IsConnectUser(UINT64 id)
{
	auto iter = mUsers.find(id);
	if (iter == mUsers.end())
		return false;

	return true;
}

User* ChatServer::FindUser(std::string_view nickname)
{
	for (auto& [id, user] : mUsers)
	{
		if (user->GetNickname() == nickname)
			return user;
	}

	return nullptr;
}

bool ChatServer::IsExistNickname(std::string_view nickname)
{
	for (auto& [id, user] : mUsers)
	{
		if (user->GetNickname() == nickname)
			return true;
	}

	return false;
}

void ChatServer::LoginHandler(Connection* connection, std::vector<std::string>* parameter)
{
	if (parameter->size() != 2)
	{
		SendRetryMessage(connection);
		return;
	}

	if (IsExistNickname((*parameter)[1]))
	{
		char message[] =
			"�ߺ��� ���̵� �Դϴ�.\r\n"
			"����> ";

		connection->Send(message, sizeof(message));
		return;
	}

	User* user = new User(connection, (*parameter)[1]);
	mUsers.insert(std::make_pair(connection->GetId(), user));

	char message[] =
		"--------------------------------------\r\n"
		"�ݰ����ϴ� �ؽ�Ʈ ä�� �����Դϴ�.\r\n"
		"�����մϴ�.\r\n"
		"--------------------------------------\r\n"
		"��ɾ�ȳ�(H) ����(X)\r\n"
		"����> ";
	
	user->Send(message, sizeof(message));
}

void ChatServer::GetCommandListHandler(User* user, std::vector<std::string>* parameter)
{
	SendShowCommandList(user);
}

void ChatServer::GetUserListHandler(User* user, std::vector<std::string>* parameter)
{
	if (parameter->size() != 1)
	{
		SendRetryMessage(user);
		return;
	}

	std::string users;
	for (auto& [id, user] : mUsers)
	{
		users += user->GetNickname();
		users += "\r\n";
	}

	char buffer[256];
	sprintf_s(buffer, users.c_str());
	user->Send(buffer, (int)users.size());	
}

void ChatServer::GetRoomListHandler(User* user, std::vector<std::string>* parameter)
{
	if (parameter->size() != 1)
	{
		SendRetryMessage(user);
		return;
	}
	
	mLobby.GetRoomList(user);
}

void ChatServer::CreateRoomHandler(User* user, std::vector<std::string>* parameter)
{
	if (parameter->size() != 3)
	{
		SendRetryMessage(user);
		return;
	}

	if (user->IsEnterRoom())
	{
		SendRetryMessage(user);
		return;
	}

	int count = 0;
	try
	{
		count = std::stoi((*parameter)[1]);

		if (count < 2)
			throw std::exception();
	}
	catch (std::exception& exception)
	{
		std::cout << exception.what() << std::endl;
		SendRetryMessage(user);
		return;
	}

	mLobby.CreateRoom(user, (*parameter)[2], count);
}

void ChatServer::JoinRoomHandler(User* user, std::vector<std::string>* parameter)
{
	if (parameter->size() != 2)
	{
		SendRetryMessage(user);
		return;
	}

	if (user->IsEnterRoom())
	{
		SendRetryMessage(user);
		return;
	}

	if(!mLobby.JoinRoom(user, (*parameter)[1]))
		SendRetryMessage(user);
}

void ChatServer::ChatHandler(User* user, char* message, int length)
{	
	if (!user->IsEnterRoom())
		return;

	char formatMessage[] = "[%s] : %s\r\n";
	char buffer[256];
	sprintf_s(buffer, formatMessage, user->GetNickname().c_str(), message);

	Room* room = user->GetRoom();
	room->Broadcast(buffer, (int)strlen(buffer));
}

void ChatServer::WhisperChatHandler(User* user, std::vector<std::string>* parameter)
{
	if (parameter->size() < 3)
	{
		SendRetryMessage(user);
		return;
	}

	std::string_view targetNickname = (*parameter)[1];
	User* targetUser = FindUser(targetNickname);
	if (targetUser == nullptr)
	{
		SendRetryMessage(user);
		return;
	}

	std::string chatMessage;
	chatMessage.reserve(256);
	for (int i = 2; i < parameter->size(); i++)
	{
		chatMessage += (*parameter)[i];
		chatMessage += " ";
	}

	char formatMessage[] = "from [%s] : %s\r\n";
	char buffer[256];
	sprintf_s(buffer, formatMessage, user->GetNickname().c_str(), chatMessage.c_str());

	targetUser->Send(buffer, (int)strlen(buffer));
}

void ChatServer::LeaveHandler(User* user, std::vector<std::string>* parameter)
{
	if (parameter->size() != 1)
	{
		SendRetryMessage(user);
		return;
	}

	if (user->IsEnterRoom())
	{
		mLobby.LeaveRoom(user, user->GetRoom());
		return;
	}
	
	UINT64 id = user->GetId();
	mUsers.erase(id);
	CloseConnection(user->GetConnection());
	
	delete user;
}