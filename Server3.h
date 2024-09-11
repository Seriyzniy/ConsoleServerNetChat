#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#define DEFAULT_PORT 7770
#define DEFAULT_IP "127.0.0.1"
#define MAX_CONNECT 5

#include <iostream>
#include <stdio.h>
#include <WinSock2.h>
#include <windows.h>
#include <thread>
#include <vector>
#include <string>

#pragma comment(lib, "Ws2_32.lib")

using namespace std;
class Server3
{
private:
	//============== D A T A   S E R V E R ==============
	SOCKET server_listen_socket = INVALID_SOCKET;
	sockaddr_in server_addr_info;

	int server_port;
	const char* server_ip;

	//============== D A T A   C L I E N T ==============
	struct client {
		char x[16];
		SOCKET client_socket = 0;
		sockaddr_in client_ip;
	};
	vector<client*> clients;
	int size_client_info = sizeof(clients[0]->client_ip);

private:
	//Функции сокета
	int Bind(SOCKET s, const sockaddr* name, int namelen);
	void Listen(SOCKET s, int backlog);
	SOCKET Accept(SOCKET s, sockaddr_in addr, int* addrlen);
	//Функции передчи данных
	char* RecvAUTO(SOCKET s, int flags);
	int SendToAUTO(SOCKET s, string msg, int flags, sockaddr* dest, int dest_len);
	//Вспомогательные функции
	void Del_connection(vector<client*>& clients, SOCKET value);
	void OutConnections();
	void Heandler(SOCKET s);
	bool ConnectedUser(char* id);
public:
	Server3(int port, const char* ip);
	~Server3();

	void start();
	void stop();
};

