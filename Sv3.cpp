#include "Server3.h"

using namespace std;

int main()
{
    WSAData wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        cerr << "WSAStartup failed\n";
        exit(1);
    }
    Server3 server(DEFAULT_PORT, DEFAULT_IP);
    server.start();
    return 0;
}