#include "Server3.h"
Server3::Server3(int port, const char* ip) {
	server_port = port;
	server_ip = ip;
	//Описание параметров сокета
	server_addr_info.sin_family = AF_INET;
	server_addr_info.sin_port = htons(server_port);
	server_addr_info.sin_addr.s_addr = inet_addr(server_ip);
	//Создание сокета
	server_listen_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (server_listen_socket == INVALID_SOCKET) {
		cout << "Create socket failed, with error: " << WSAGetLastError() << endl;
		Server3::~Server3();
		exit(1);
	}
}
Server3::~Server3() {
	closesocket(server_listen_socket);
	for (int i = 0; ; i++) {
		if (clients.empty()) {
			break;
		}
		closesocket(clients[i]->client_socket);
		WSACleanup();
	}
}

int Server3::Bind(SOCKET s, const sockaddr* addr, int namelen) {
	int res = bind(s, addr, namelen);
	if (res != 0) {
		cout << "Bind failed, with error: " << WSAGetLastError() << endl;
		Server3::~Server3();
		exit(1);
	}
	return res;
}
void Server3::Listen(SOCKET s, int backlog) {
	if (listen(s, backlog) == SOCKET_ERROR) {
		cout << "Listen failed, with error: " << WSAGetLastError() << endl;
		Server3::~Server3();
		exit(1);
	}
}
SOCKET Server3::Accept(SOCKET s, sockaddr_in addr, int* addrlen) {
	client *newconnect = new client;
	newconnect->client_socket = accept(s, (SOCKADDR*)&addr, addrlen);
	newconnect->client_ip = addr;

	if (newconnect->client_socket == INVALID_SOCKET) {
		cout << "Accept failed, with error: " << WSAGetLastError() << endl;
		Server3::~Server3();
		exit(1);
	}
	cout << "Client connected: " << inet_ntoa(addr.sin_addr) << " : " << addr.sin_port << endl;

	//Добавление ID соединения и инфу о клиенте в массив
	clients.push_back(newconnect);

	return newconnect->client_socket;
}

char* Server3::RecvAUTO(SOCKET s, int flags) {
	int size_msg = 0;
	int recv_block = 0;
	//Принятие длины сообщения
	recv_block = recv(s, (char*)&size_msg, sizeof(int), 0);
	size_msg = atoi((char*)&size_msg);
	
	//Подготовка буфера для принятия сообщения
	char* buff = new char[size_msg + 1];
	buff[size_msg] = '\0';

	//Принятие сообщения
	recv_block = recv(s, buff, size_msg, flags);
	if (recv_block == SOCKET_ERROR) {
		return nullptr;
	}
	return buff;
}
int Server3::SendToAUTO(SOCKET s, string msg, int flags, sockaddr* dest, int dest_len) {
	int res;
	int msg_size = msg.size();
	//Отправка длины сообщения
	sendto(s, to_string(msg_size).c_str(), sizeof(int), flags, dest, dest_len);
	//Отправка сообщения
	res = sendto(s, msg.c_str(), msg_size, flags, dest, dest_len);
	if (res == SOCKET_ERROR) {
		return res;
	}
	return res;
}

void Server3::Del_connection(vector<client*>& clients, SOCKET value) {
	vector<client*>::iterator iter;
	for (iter = clients.begin(); iter != clients.end(); iter++) {
		if ((**iter).client_socket == value) {
			shutdown((**iter).client_socket, 2);
			closesocket((**iter).client_socket);
			clients.erase(iter);
			cout << "Disconnect client " << value << endl;
			break;
		}
	}
}
void Server3::OutConnections() {
	vector<client*>::iterator iter = clients.begin();
	for (iter; iter != clients.end(); iter++) {
		cout << (**iter).client_socket << '\t' << inet_ntoa((**iter).client_ip.sin_addr) << '\t' << (**iter).client_ip.sin_port << endl;
	}
}
bool Server3::ConnectedUser(char* id) {
	for (client* p : clients) {
		if (atoi(id) == p->client_socket) {
			return true;
		}
	}
	return false;
}

void Server3::Heandler(SOCKET s) {
	string hello_msg = "(1) Get a list of connections\n(2) Open chat\n(3) Disconnect";
	string message;
	char *buff = new char[256];
	char* id;

	while (true) {
		SendToAUTO(s, hello_msg, 0, 0, 0);
		buff = RecvAUTO(s, 0);
		if (!buff) {
			Del_connection(clients, s);
			break;
		}
		switch (*buff) {
		case '1':
			for (int i = 0; i < clients.size(); i++) {
				if (s == clients[i]->client_socket) {
					continue;
				}
				message.append(to_string(clients[i]->client_socket)).append("\t").
					append(inet_ntoa(clients[i]->client_ip.sin_addr)).append(" : ").
					append(to_string(clients[i]->client_ip.sin_port)).append("\n");
			}
			if (message.empty()) {
				SendToAUTO(s, "No connections\0", 0, NULL, 0);
				break;
			}
			SendToAUTO(s, message, 0, 0, 0);
			message.erase(0, message.size());
			break;
		case '2':
			SendToAUTO(s, "Select a user ID",0,0,0);
			id = RecvAUTO(s, 0);
			if (!id) {
				Del_connection(clients, s);
				break;
			}
			else
				if (ConnectedUser(id)) {
					while (true) {
						buff = RecvAUTO(s, 0);
						if (!buff) {
							Del_connection(clients, s);
							break;
						}
						if (*buff == 'q') {
							break;
						}
						SendToAUTO(atoi(id), buff, 0, 0, 0);
					}
				}
				else SendToAUTO(s, "User was not found!", 0, 0, 0);
			break;
		case '3':
			Del_connection(clients, s);
			break;
		default: SendToAUTO(s, "Error options!", 0, 0, 0);
		}
	}
	
}
void Server3::start() {
	SOCKET tmp;
	sockaddr_in client = { 0 };

	Bind(server_listen_socket, (SOCKADDR*)&server_addr_info, sizeof(server_addr_info));
	cout << "Listening...\n";
	Listen(server_listen_socket, MAX_CONNECT);

	while (true) {
		tmp = Accept(server_listen_socket, client, &size_client_info);
		OutConnections();
		thread t(&Server3::Heandler, this, tmp);
		t.detach();
	}
}