#include <iostream>
#include <winsock2.h>

#pragma comment(lib,"ws2_32.lib") // Winsock Library
#pragma warning(disable:4996) 

#define BUFLEN 512
#define PORT 90

#include "src/httpLib.h"

struct Connection {
    enum class Type {
        LeftRad,
        RightRad,
        FrontRad
    };
    std::string ip;
    Type type;
    httplib::Client *cli;
};

std::vector<Connection> connections;

bool handleUDPRunning;
std::thread* udpHandler;
int handleUDP() {
    handleUDPRunning = true;
    sockaddr_in server, client;

    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        return 1;
    }

    SOCKET server_socket;
    if ((server_socket = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
    {
        return 2;
    }

    // prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    // bind
    if (bind(server_socket, (sockaddr*)&server, sizeof(server)) == SOCKET_ERROR)
    {
        return 3;
    }

    while (handleUDPRunning)
    {
        char message[BUFLEN] = {};

        int message_len;
        int slen = sizeof(sockaddr_in);
        if (message_len = recvfrom(server_socket, message, BUFLEN, 0, (sockaddr*)&client, &slen) == SOCKET_ERROR)
        {
            return 4;
        }

        bool found = false;
        for (int i = 0; i < connections.size() && !found; i++)
            if (connections[i].ip == std::string(inet_ntoa(client.sin_addr)))
                found = true;
        if (found)
            continue;
        connections.push_back({});
        connections[connections.size() - 1].ip = std::string(inet_ntoa(client.sin_addr));
        connections[connections.size() - 1].cli = new httplib::Client(connections[connections.size() - 1].ip.c_str());
        if (std::string(message) == "Hi I Am Left")
            connections[connections.size() - 1].type = Connection::Type::LeftRad;
        if (std::string(message) == "Hi I Am Right")
            connections[connections.size() - 1].type = Connection::Type::RightRad;
        if (std::string(message) == "Hi I Am Front")
            connections[connections.size() - 1].type = Connection::Type::FrontRad;
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
    // start Thread for UDP listen;
        // make connection for each klient
        // ip + type

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
                auto res = connections[i].cli->Get("/Forward?distance=5000&speed=125");
            }
            break;
        case 2:
            for (int i = 0; i < connections.size(); i++) {
                auto res = connections[i].cli->Get("/Backward?distance=5000&speed=125");
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