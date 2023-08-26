#include <iostream>
#include <winsock2.h>

#pragma comment(lib,"ws2_32.lib") // Winsock Library
#pragma warning(disable:4996) 

#define BUFLEN 512
#define PORT 90

#include "src/httpLib.h"

struct Connection {
    enum class Type : uint16_t {
        linkesRad = 1,
        rechteRad = 2,
        vorderRad = 3,
        lenkRad = 4
    };
    bool exists = false;
    std::string ip = "";
    uint16_t refreshTime = 0;
    double empfangszeit = 0;
    httplib::Client *cli = nullptr;

    void Forward() {
        cli->Get("/Forward?steps=500&speed=150");
        std::cout << "go" << std::endl;

    }
    void Backward() {
        cli->Get("/Forward?steps=500&speed=50");
    }
    void Stop() {
        cli->Get("/Stop");
    }
};

Connection linkesRad = { false, "", 0, 0, nullptr };
Connection vorderRad = { false, "", 0, 0, nullptr };
Connection rechtenRad = { false, "", 0, 0, nullptr };
Connection lenkRad = { false, "", 0, 0, nullptr };

time_t start;

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

    while (handleUDPRunning) {
        char message[BUFLEN] = {};

        int message_len;
        int slen = sizeof(sockaddr_in);
        if (message_len = recvfrom(server_socket, message, BUFLEN, 0, (sockaddr*)&client, &slen) == SOCKET_ERROR) return 4;

        std::string msg = std::string(message);
        if ((int)msg.find(',') == -1) continue;

        std::string msg0 = msg.substr(0, msg.find_first_of(','));
        std::string msg1 = msg.substr(msg.find_first_of(',') + 1);
        uint16_t typeId = std::stoi(msg0.substr(msg0.find_first_of('=') + 1));
        uint16_t refreshTime = std::stoi(msg1.substr(msg1.find_first_of('=') + 1));

        switch (typeId)
        {
        case (uint16_t)Connection::Type::linkesRad: {
            if (linkesRad.cli == nullptr) {
                linkesRad.ip = std::string(inet_ntoa(client.sin_addr));
                linkesRad.cli = new httplib::Client(linkesRad.ip.c_str());
                linkesRad.exists = true;
            }
            linkesRad.refreshTime = refreshTime;
            linkesRad.empfangszeit = difftime(time(0), start);
        }
            break;
        case (uint16_t)Connection::Type::rechteRad: {
            if (rechtenRad.cli == nullptr) {
                rechtenRad.ip = std::string(inet_ntoa(client.sin_addr));
                rechtenRad.cli = new httplib::Client(rechtenRad.ip.c_str());
                rechtenRad.exists = true;
            }
            rechtenRad.refreshTime = refreshTime;
            rechtenRad.empfangszeit = difftime(time(0), start);
        }
            break;
        case (uint16_t)Connection::Type::vorderRad: {
            if (vorderRad.cli == nullptr) {
                vorderRad.ip = std::string(inet_ntoa(client.sin_addr));
                vorderRad.cli = new httplib::Client(vorderRad.ip.c_str());
                vorderRad.exists = true;
            }
            vorderRad.refreshTime = refreshTime;
            vorderRad.empfangszeit = difftime(time(0), start);
        }
            break;
        case (uint16_t)Connection::Type::lenkRad: {
            if (lenkRad.cli == nullptr) {
                lenkRad.ip = std::string(inet_ntoa(client.sin_addr));
                lenkRad.cli = new httplib::Client(lenkRad.ip.c_str());
                lenkRad.exists = true;
            }
            lenkRad.refreshTime = refreshTime;
            lenkRad.empfangszeit = difftime(time(0), start);
        }
            break;
        default:
            std::cout << "Hä " << typeId << std::endl;
            break;
        }
    }

    closesocket(server_socket);
    WSACleanup();
    handleUDPRunning = false;
}
void endHandleUDP() {
    handleUDPRunning = false;
    udpHandler->join();
}

enum State {
    F,
    L,
    R,
    S
};
State state = State::S;
bool move;

httplib::Server serv;

int main()
{
    start = time(0);

    serv.Get("/", [](const httplib::Request& req, httplib::Response& res) {
        std::ifstream inFile("Web/index.html");
        std::string data;
        char buf[1024];
        while (!inFile.eof()) {
            inFile.getline(buf, 1024);
            data += buf;
            data += '\n';
        }
        res.set_content(data, "text/html");
        });
    serv.Get("/State", [](const httplib::Request& req, httplib::Response& res) {
        if (vorderRad.cli == nullptr || lenkRad.cli == nullptr)
            res.body = "Not Connected";
        else
            res.body = "Connected";
        });
    serv.Get("/Fornt", [](const httplib::Request& req, httplib::Response& res) {
        state = State::F;
        res.body = "F";
        });
    serv.Get("/Left", [](const httplib::Request& req, httplib::Response& res) {
        state = State::L;
        res.body = "L";
        });
    serv.Get("/Right", [](const httplib::Request& req, httplib::Response& res) {
        state = State::R;
        res.body = "R";
        });
    serv.Get("/Stop", [](const httplib::Request& req, httplib::Response& res) {
        move = false;
        res.body = "S";
        });
    serv.Get("/Move", [](const httplib::Request& req, httplib::Response& res) {
        if (state != State::S) return;
        res.body = "M";
        move = true;
        });

    udpHandler = new std::thread(handleUDP);
    std::thread webHandler = std::thread([]() {
        char host[256];
        char* IP;
        struct hostent* host_entry;
        int hostname;
        hostname = gethostname(host, sizeof(host));
        host_entry = gethostbyname(host);
        IP = inet_ntoa(*((struct in_addr*)host_entry->h_addr_list[0]));
        std::cout << IP << std::endl;
        serv.listen(IP, 80);
        });

    bool notStopped = true;
    while (notStopped) {
        if (vorderRad.exists && lenkRad.exists) {

            switch (state)
            {
            case F:
            {
                std::string temp = lenkRad.cli->Get("/State")->body;
                if (temp.substr(0, 1) != "0") {
                    vorderRad.Stop();
                    //linkesRad.Stop();
                    //rechtenRad.Stop();
                    lenkRad.cli->Get("/Direction?state=0");
                }
                else {
                    state = State::S;
                }
                break;
            }
            case L:
            {
                std::string temp = lenkRad.cli->Get("/State")->body;
                if (temp.substr(0, 2) != "90") {
                    vorderRad.Stop();
                    //linkesRad.Stop();
                    //rechtenRad.Stop();
                    lenkRad.cli->Get("/Direction?state=90");
                }
                else {
                    state = State::S;
                }
                break;
            }
            case R:
            {
                std::string temp = lenkRad.cli->Get("/State")->body;
                if (temp.substr(0, 3) != "-90") {
                    vorderRad.Stop();
                    //linkesRad.Stop();
                    //rechtenRad.Stop();
                    lenkRad.cli->Get("/Direction?state=-90");
                }
                else {
                    state = State::S;
                }
                break;
            }
            case S:
            {
                lenkRad.Stop();
                break;
            }
            default:
                notStopped = false;
                break;
            }

            if (move) {
                vorderRad.Forward();
            }
            else {
                vorderRad.Stop();
            }
        }
    }

    serv.stop();
    webHandler.join();
    endHandleUDP();
    return 0;
}