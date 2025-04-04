#include <iostream>
#include <unordered_map>
#include <nlohmann/json.hpp>
#include <enet/enet.h>
#include <curl/curl.h>
#include <cstring>
#include <cstdlib>
#include <string>
#include <array>
#include <stdexcept>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <thread>
/*
使用要求：
要求主机和客户端处于同一局域网下（能通过ip地址相互访问），远程联机需要使用内网穿透
内网穿透实现：我使用Zerotier将两台主机处于同一虚拟网络中，并使用虚拟IP实现相互沟通
使用方法：
    runServer(GridType& grid)用于服务器端的开始服务，每5s发送心跳包证明在线，并接受客户端的操作和心跳包
    runClient(const char* IP,GridType& grid,json change)用于客户端的开始服务，每5s发送心跳包证明在线，并接受服务器端的生命状态和心跳包
    服务器和客户端的接收发送功能皆为在其他线程实现，但会阻塞主线程，调用函数时需要在新线程调用
    开始服务后，使用send()函数即可实现数据发送
*/
using namespace std; 
using json = nlohmann::json;
using INT = int;
using GridType = std::unordered_map<INT, std::unordered_map<INT, BOOL>>;
bool go=true;
bool ifSend = false;

// 将 grid 转换为 JSON
static json serializeGrid(const GridType& grid) {
    json j;
    for (const auto& [y, row] : grid) {
        for (const auto& [x, value] : row) {
            j[std::to_string(y)][std::to_string(x)] = value;
        }
    }
    return j;
}

// 反序列化 JSON 到 grid
static GridType deserializeGrid(const json& j) {
    GridType grid;
    for (const auto& [yStr, row] : j.items()) {
        INT y = std::stoi(yStr);
        for (const auto& [xStr, value] : row.items()) {
            INT x = std::stoi(xStr);
            grid[y][x] = value.get<BOOL>();
        }
    }
    return grid;
}

void Send()
{
    ifSend = true;
}

void serverSendLoop(ENetPeer* peer,GridType &grid) {
    while (go) {
        if (!ifSend)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            continue;
        }
        json data = serializeGrid(grid);
        std::string jsonData = data.dump();
        ENetPacket* packet = enet_packet_create(jsonData.c_str(), jsonData.size() + 1, ENET_PACKET_FLAG_RELIABLE);
        enet_peer_send(peer, 0, packet);
        enet_host_flush(peer->host);
        ifSend = false;

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void serverBeatSendLoop(ENetPeer* peer) {
    while (go) {
        ENetPacket* packet = enet_packet_create("go", 3, ENET_PACKET_FLAG_RELIABLE);
        enet_peer_send(peer, 0, packet);
        enet_host_flush(peer->host);
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
}

void serverReceiveLoop(ENetHost* server) {
    ENetEvent event;
    int waittime;
    while (go) {
        while (enet_host_service(server, &event, 100) > 0) {
            switch (event.type) {
            case ENET_EVENT_TYPE_RECEIVE: {
                std::string receivedData(reinterpret_cast<char*>(event.packet->data));
                std::cout << "收到客户端数据: " << receivedData << std::endl;
                
                if (receivedData != "go")
                {
                    //添加对客户操作数据包的处理

                }
                
                enet_packet_destroy(event.packet);
                waittime = 0;
                break;
            }
            case ENET_EVENT_TYPE_DISCONNECT:

                std::cout << "客户端断开连接。" << std::endl;
                return;
            default:
                break;
            }
            waittime++;
        }
        if (waittime > 200)go=false; //20s未收到数据包则认为断联
    }
}

void runServer(GridType& grid) {
    ENetAddress address;
    address.host = ENET_HOST_ANY;
    address.port = 12138;

    ENetHost* server = enet_host_create(&address, 1, 2, 0, 0);
    if (server == nullptr) {
        std::cerr << "服务器初始化失败！" << std::endl;
        exit(EXIT_FAILURE);
    }

    std::cout << "服务器启动，监听端口 12138，等待客户端连接..." << std::endl;

    ENetEvent event;
    if (enet_host_service(server, &event, 50000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT) {
        std::cout << "客户端已连接。" << std::endl;
    }
    else {
        std::cerr << "客户端连接超时。" << std::endl;
        enet_host_destroy(server);
        return;
    }

    // 等待客户端发送 "准备" 指令
    while (true) {
        if (enet_host_service(server, &event, 10000) > 0 && event.type == ENET_EVENT_TYPE_RECEIVE) {
            std::string message(reinterpret_cast<char*>(event.packet->data));
            if (message == "准备") {
                std::cout << "收到客户端 '准备' 指令，进入主循环。" << std::endl;
                enet_packet_destroy(event.packet);
                go = true;
                break;
            }
        }
    }

    // 启动服务器的发送和接收线程
    std::thread beatThread(serverBeatSendLoop, event.peer);
    std::thread sendThread(serverSendLoop, event.peer,grid);
    std::thread receiveThread(serverReceiveLoop, server);

    beatThread.join();
    sendThread.join();
    receiveThread.join();

    enet_host_destroy(server);
}

void clientSendLoop(ENetPeer* peer,json& change) {
    while (go) {
        if (!ifSend)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            continue;
        }

        std::string jsonData = change.dump(); //用户的设置数据
        ENetPacket* packet = enet_packet_create(jsonData.c_str(), jsonData.size() + 1, ENET_PACKET_FLAG_RELIABLE);
        enet_peer_send(peer, 0, packet);
        enet_host_flush(peer->host);
        std::cout << "客户端发送操作包。" << std::endl;
        ifSend = false;
    }
}

void clientBeatSendLoop(ENetPeer* peer) {
    while (go) {
        ENetPacket* packet = enet_packet_create("go", 3, ENET_PACKET_FLAG_RELIABLE);
        enet_peer_send(peer, 0, packet);
        enet_host_flush(peer->host);
        std::cout << "客户端发送心跳包。" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
}

void clientReceiveLoop(ENetHost* client,GridType& grid) {
    ENetEvent event;
    int waittime;
    while (go) {
        while (enet_host_service(client, &event, 100) > 0) {
            if (event.type == ENET_EVENT_TYPE_RECEIVE) {
                std::string receivedData(reinterpret_cast<char*>(event.packet->data));
                if (receivedData != "go")
                {
                    json newdata = json::parse(receivedData);
                    grid = deserializeGrid(newdata);
                }
                std::cout << "收到服务器数据: " << receivedData << std::endl;
                waittime = 0;
                enet_packet_destroy(event.packet);
            }
            else if (event.type == ENET_EVENT_TYPE_DISCONNECT) {
                go = false;
                std::cout << "与服务器断开连接。" << std::endl;
                return;
            }
        }
        waittime++;
        if (waittime > 200)go = false;
    }
}

void runClient(const char* serverIP,GridType& grid,json& change) {
    ENetHost* client = enet_host_create(nullptr, 1, 2, 0, 0);
    if (client == nullptr) {
        std::cerr << "客户端初始化失败！" << std::endl;
        exit(EXIT_FAILURE);
    }

    ENetAddress address;
    enet_address_set_host(&address, serverIP);
    address.port = 12138;

    ENetPeer* peer = enet_host_connect(client, &address, 2, 0);
    if (peer == nullptr) {
        std::cerr << "无法创建连接！" << std::endl;
        enet_host_destroy(client);
        exit(EXIT_FAILURE);
    }

    ENetEvent event;
    if (enet_host_service(client, &event, 5000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT) {
        std::cout << "成功连接到服务器: " << serverIP << std::endl;
    }
    else {
        std::cerr << "连接失败或超时。" << std::endl;
        enet_peer_reset(peer);
        enet_host_destroy(client);
        exit(EXIT_FAILURE);
    }

    // 发送 "准备" 指令
    const char* readyMsg = "准备";
    ENetPacket* packet = enet_packet_create(readyMsg, std::strlen(readyMsg) + 1, ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send(peer, 0, packet);
    enet_host_flush(client);
    go = true;

    // 启动发送和接收线程
    std::thread beatThread(clientBeatSendLoop,peer);
    std::thread sendThread(clientSendLoop, peer,change);
    std::thread receiveThread(clientReceiveLoop, client,grid);

    beatThread.join();
    sendThread.join();
    receiveThread.join();

    enet_host_destroy(client);
}

/*
int main(int argc, char* argv[]) {
    // 初始化ENet
    GridType grid;
    json change;
    grid[1][1] = 1;
    if (enet_initialize() != 0) {
        std::cerr << "ENet初始化失败！" << std::endl;
        return EXIT_FAILURE;
    }
    atexit(enet_deinitialize);

    int mode;
    cout << "输入模式：1主机2客户端：" << endl;
    cin >> mode;
    if (mode == 1) {
        runServer(grid);
    }
    else if (mode == 2) {
        string IP;
        cout << "输入连接的IP：" << endl;
        cin >> IP;
        runClient(IP.c_str(),grid,change);
    }
    else {
        std::cerr << "未知的模式，请使用 1 或 2。" << std::endl;
        return EXIT_FAILURE; 
    }

    return EXIT_SUCCESS;
}
*/
