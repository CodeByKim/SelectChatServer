#pragma once

#include "Common.h"
#include "Server.h"
#include "Lobby.h"

#define COMMAND_ERROR			-1
#define COMMAND_LOGIN			 0
#define COMMAND_GET_USER_LIST	 1
#define COMMAND_GET_ROOM_LIST	 2
#define COMMAND_CREATE_ROOM		 3
#define COMMAND_JOIN_ROOM		 4
#define COMMAND_WHISPER_CHAT	 5
#define COMMAND_GET_COMMAND_LIST 6
#define COMMAND_CHAT			 7
#define COMMAND_LEAVE			 8

class Connection;
class User;

class ChatServer : public Server
{
public:
	ChatServer();
	~ChatServer();

public:
	void  OnAccept(ConnectionEvent& connectionEvent)                override;
	void  OnReceive(ConnectionEvent& connectionEvent, char* packet) override;
	void  OnDisconnect(ConnectionEvent& connectionEvent)            override;

private:
	int   GetCommand(User* user, std::string& command);
	void  ParsePacket(Connection* connection, char* packet);
	void  SendRetryMessage(Connection* connection);
	void  SendRetryMessage(User* user);
	void  SendShowCommandList(User* user);

	bool  IsConnectUser(UINT64 id);
	User* FindUser(std::string_view nickname);
	bool  IsExistNickname(std::string_view nickname);

	void  LoginHandler(Connection* connection, std::vector<std::string>* parameter);
	void  GetCommandListHandler(User* user, std::vector<std::string>* parameter);
	void  GetUserListHandler(User* user, std::vector<std::string>* parameter);
	void  GetRoomListHandler(User* user, std::vector<std::string>* parameter);
	void  CreateRoomHandler(User* user, std::vector<std::string>* parameter);
	void  JoinRoomHandler(User* user, std::vector<std::string>* parameter);
	void  ChatHandler(User* user, char* message, int length);
	void  WhisperChatHandler(User* user, std::vector<std::string>* parameter);
	void  LeaveHandler(User* user, std::vector<std::string>* parameter);

	std::unordered_map<UINT64, User*> mUsers;
	Lobby                             mLobby;
};