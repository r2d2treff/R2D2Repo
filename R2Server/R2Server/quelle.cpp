#include <iostream>
#include <winsock2.h>

#pragma comment(lib,"ws2_32.lib") // Winsock Library
#pragma warning(disable:4996) 

#define BUFLEN 512
#define PORT 90

#include "src/httpLib.h"

struct Connection {
    enum class Type : uint16_t {
        LeftRad = 1,
        RightRad = 2,
        FrontRad = 3
    };
    std::string ip;
    Type type;
    uint16_t refreshTime;
    double empfangszeit;
    httplib::Client *cli;
};

time_t start;
std::vector<Connection> connections;

bool handleUDPRunning;
std::thread* udpHandler;
int handleUDP() {
    handleUDPRunning = true;
    sockaddr_in server, client;

    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return 1;

    SOCKET server_socket;
    if ((server_socket = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET) return 2;

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    if (bind(server_socket, (sockaddr*)&server, sizeof(server)) == SOCKET_ERROR) return 3;

    while (handleUDPRunning)
    {
        char message[BUFLEN] = {};

        int message_len;
        int slen = sizeof(sockaddr_in);
        if (message_len = recvfrom(server_socket, message, BUFLEN, 0, (sockaddr*)&client, &slen) == SOCKET_ERROR) return 4;

        std::string msg = std::string(message);
        if ((int)msg.find(',') == -1) continue;

        int found = -1;
        for (int i = 0; i < connections.size() && found == -1; i++)
            if (connections[i].ip == std::string(inet_ntoa(client.sin_addr)))
                found = i;
        if (found == -1) {
            found = connections.size();
            connections.push_back({});
            connections[found].ip = std::string(inet_ntoa(client.sin_addr));
            connections[found].cli = new httplib::Client(connections[found].ip.c_str());
        }

        std::string msg0 = msg.substr(0, msg.find_first_of(','));
        std::string msg1 = msg.substr(msg.find_first_of(',') + 1);
        uint16_t typeId = std::stoi(msg0.substr(msg0.find_first_of('=') + 1));
        uint16_t refreshTime = std::stoi(msg1.substr(msg1.find_first_of('=') + 1));

        connections[found].type = (Connection::Type)typeId;
        connections[found].refreshTime = refreshTime;
        connections[found].empfangszeit = difftime(time(0), start);
    }

    closesocket(server_socket);
    WSACleanup();
    handleUDPRunning = false;
}
void endHandleUDP() {
    handleUDPRunning = false;
    udpHandler->join();
}

int main()
{
    start = time(0);
    udpHandler = new std::thread(handleUDP);

    bool stop = false;
    while (!stop) {
        int temp;
        std::cin >> temp;

        switch (temp)
        {
        case 0:
            for (int i = 0; i < connections.size(); i++) {
                auto res = connections[i].cli->Get("/Stop");
            }
            break;
        case 1:
            for (int i = 0; i < connections.size(); i++) {
                auto res = connections[i].cli->Get("/Forward?distance=1000&speed=125");
            }
            break;
        case 2:
            for (int i = 0; i < connections.size(); i++) {
                auto res = connections[i].cli->Get("/Backward?distance=1000&speed=125");
            }
            break;
        case 3:
            stop = true;
            break;
        default:
            break;
        }

    }

    endHandleUDP();
    for (int i = 0; i < connections.size(); i++) {
        auto res = connections[i].cli->Get("/Stop");
    }
}