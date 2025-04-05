#include "framework.h"
#include <iostream>
#include <nlohmann/json.hpp>
#include <enet/enet.h>
#include <cstring>
#include <cstdlib>
#include <thread>
#include <chrono>
#include <atomic>
#include <stdexcept>
// Windows 平台可能需要包含 winsock2.h 等头文件
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#pragma comment(lib, "winmm.lib")

using namespace std;
using json = nlohmann::json;
using INT = int;
using GridType = std::unordered_map<INT, std::unordered_map<INT, BOOL>>;

// 全局控制变量（使用原子类型保证线程安全）
atomic<bool> go(true);
atomic<bool> ifsend(false);

// 将 grid 转换为 JSON
static json serializeGrid(const GridType& grid) {
    json j;
    for (const auto& [y, row] : grid) {
        for (const auto& [x, value] : row) {
            j[to_string(y)][to_string(x)] = value;
        }
    }
    return j;
}

// 反序列化 JSON 到 grid
static GridType deserializeGrid(const json& j) {
    GridType grid;
    for (const auto& [yStr, row] : j.items()) {
        INT y = stoi(yStr);
        for (const auto& [xStr, value] : row.items()) {
            INT x = stoi(xStr);
            grid[y][x] = value.get<BOOL>();
        }
    }
    return grid;
}

void mySendMessage() {
    ifsend = true;
}

void serverSendLoop(ENetPeer* peer, GridType& grid) {
    while (go) {
        if (!ifsend) {
            this_thread::sleep_for(chrono::seconds(1));
            continue;
        }
        json data = serializeGrid(grid);
        string jsonData = data.dump();
        ENetPacket* packet = enet_packet_create(jsonData.c_str(), jsonData.size() + 1, ENET_PACKET_FLAG_RELIABLE);
        enet_peer_send(peer, 0, packet);
        enet_host_flush(peer->host);
        ifsend = false;
        this_thread::sleep_for(chrono::seconds(1));
    }
}

void serverBeatSendLoop(ENetPeer* peer) {
    while (go) {
        ENetPacket* packet = enet_packet_create("go", 3, ENET_PACKET_FLAG_RELIABLE);
        enet_peer_send(peer, 0, packet);
        enet_host_flush(peer->host);
        this_thread::sleep_for(chrono::seconds(5));
    }
}

void serverReceiveLoop(ENetHost* server) {
    ENetEvent event;
    int idleCounter = 0;
    while (go) {
        int eventCount = enet_host_service(server, &event, 100);
        if (eventCount > 0) {
            idleCounter = 0;
            do {
                switch (event.type) {
                case ENET_EVENT_TYPE_RECEIVE: {
                    string receivedData(reinterpret_cast<char*>(event.packet->data));
                    cout << "收到客户端数据: " << receivedData << endl;
                    if (receivedData != "go") {
                        // 添加对客户端操作数据包的处理
                    }
                    enet_packet_destroy(event.packet);
                    break;
                }
                case ENET_EVENT_TYPE_DISCONNECT:
                    cout << "客户端断开连接。" << endl;
                    go = false;
                    break;
                default:
                    break;
                }
            } while (enet_host_service(server, &event, 0) > 0);
        }
        else {
            idleCounter++;
        }
        if (idleCounter > 200) { // 超过20秒未收到数据包则认为断联
            go = false;
        }
    }
}

BOOL runServer(GridType& grid) {
    ENetAddress address;
    address.host = ENET_HOST_ANY;
    address.port = 12138;

    ENetHost* server = enet_host_create(&address, 1, 2, 0, 0); // 此处有问题，什么host.c，
                                                               // 疑似/lib里enet.lib差东西?
    if (server == nullptr) {
        cerr << "服务器初始化失败！" << endl;
        return FALSE;// exit(EXIT_FAILURE);
    }

    cout << "服务器启动，监听端口 12138，等待客户端连接..." << endl;

    ENetEvent event;
    if (enet_host_service(server, &event, 50000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT) {
        cout << "客户端已连接。" << endl;
    }
    else {
        cerr << "客户端连接超时。" << endl;
        enet_host_destroy(server);
        return FALSE;
    }

    // 等待客户端发送 "准备" 指令
    while (true) {
        if (enet_host_service(server, &event, 10000) > 0 && event.type == ENET_EVENT_TYPE_RECEIVE) {
            string message(reinterpret_cast<char*>(event.packet->data));
            if (message == "准备") {
                cout << "收到客户端 '准备' 指令，进入主循环。" << endl;
                enet_packet_destroy(event.packet);
                go = true;
                break;
            }
        }
    }

    // 启动服务器的发送和接收线程
    thread beatThread(serverBeatSendLoop, event.peer);
    thread sendThread(serverSendLoop, event.peer, ref(grid));
    thread receiveThread(serverReceiveLoop, server);

    beatThread.join();
    sendThread.join();
    receiveThread.join();

    enet_host_destroy(server);
    return TRUE;
}

void clientSendLoop(ENetPeer* peer, json& change) {
    while (go) {
        if (!ifsend) {
            this_thread::sleep_for(chrono::seconds(1));
            continue;
        }
        string jsonData = change.dump(); // 用户的设置数据
        ENetPacket* packet = enet_packet_create(jsonData.c_str(), jsonData.size() + 1, ENET_PACKET_FLAG_RELIABLE);
        enet_peer_send(peer, 0, packet);
        enet_host_flush(peer->host);
        cout << "客户端发送操作包。" << endl;
        ifsend = false;
        this_thread::sleep_for(chrono::seconds(1)); // 防止忙等待
    }
}

void clientBeatSendLoop(ENetPeer* peer) {
    while (go) {
        ENetPacket* packet = enet_packet_create("go", 3, ENET_PACKET_FLAG_RELIABLE);
        enet_peer_send(peer, 0, packet);
        enet_host_flush(peer->host);
        cout << "客户端发送心跳包。" << endl;
        this_thread::sleep_for(chrono::seconds(5));
    }
}

void clientReceiveLoop(ENetHost* client, GridType& grid) {
    ENetEvent event;
    int idleCounter = 0;
    while (go) {
        int eventCount = enet_host_service(client, &event, 100);
        if (eventCount > 0) {
            idleCounter = 0;
            do {
                if (event.type == ENET_EVENT_TYPE_RECEIVE) {
                    string receivedData(reinterpret_cast<char*>(event.packet->data));
                    if (receivedData != "go") {
                        try {
                            json newdata = json::parse(receivedData);
                            grid = deserializeGrid(newdata);
                        }
                        catch (std::exception& e) {
                            cerr << "解析JSON数据出错: " << e.what() << endl;
                        }
                    }
                    cout << "收到服务器数据: " << receivedData << endl;
                    enet_packet_destroy(event.packet);
                }
                else if (event.type == ENET_EVENT_TYPE_DISCONNECT) {
                    go = false;
                    cout << "与服务器断开连接。" << endl;
                    break;
                }
            } while (enet_host_service(client, &event, 0) > 0);
        }
        else {
            idleCounter++;
        }
        if (idleCounter > 200) { // 超过20秒无数据认为断联
            go = false;
        }
    }
}

BOOL runClient(const char* serverIP, GridType& grid, json& change) {
    ENetHost* client = enet_host_create(nullptr, 1, 2, 0, 0);
    if (client == nullptr) {
        cerr << "客户端初始化失败！" << endl;
        return FALSE;// exit(EXIT_FAILURE);
    }

    ENetAddress address;
    enet_address_set_host(&address, serverIP);
    address.port = 12138;

    ENetPeer* peer = enet_host_connect(client, &address, 2, 0);
    if (peer == nullptr) {
        cerr << "无法创建连接！" << endl;
        enet_host_destroy(client);
        return FALSE; // exit(EXIT_FAILURE);
    }

    ENetEvent event;
    if (enet_host_service(client, &event, 5000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT) {
        cout << "成功连接到服务器: " << serverIP << endl;
    }
    else {
        cerr << "连接失败或超时。" << endl;
        enet_peer_reset(peer);
        enet_host_destroy(client);
        return FALSE; // exit(EXIT_FAILURE);
    }

    // 发送 "准备" 指令
    const char* readyMsg = "准备";
    ENetPacket* packet = enet_packet_create(readyMsg, std::strlen(readyMsg) + 1, ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send(peer, 0, packet);
    enet_host_flush(client);
    go = true;

    // 启动发送和接收线程
    thread beatThread(clientBeatSendLoop, peer);
    thread sendThread(clientSendLoop, peer, ref(change));
    thread receiveThread(clientReceiveLoop, client, ref(grid));

    beatThread.join();
    sendThread.join();
    receiveThread.join();

    enet_host_destroy(client);
    return TRUE;
}

/*
int main(int argc, char* argv[]) {
    // 初始化ENet
    GridType grid;
    json change;
    change["test"] = "111";
    grid[1][1] = 1;

    if (enet_initialize() != 0) {
        cerr << "ENet初始化失败！" << endl;
        return EXIT_FAILURE;
    }
    atexit(enet_deinitialize);

    int mode;
    cout << "输入模式：1主机 2客户端：" << endl;
    cin >> mode;

    thread networkThread;
    if (mode == 1) {
        // 传引用时需使用 std::ref
        networkThread = thread(runServer, ref(grid));
    }
    else if (mode == 2) {
        string IP;
        cout << "输入连接的IP：" << endl;
        cin >> IP;
        networkThread = thread(runClient, IP.c_str(), ref(grid), ref(change));
    }
    else {
        cerr << "未知的模式，请使用 1 或 2。" << endl;
        return EXIT_FAILURE;
    }

    // 模拟主线程在10秒后触发发送操作
    this_thread::sleep_for(chrono::seconds(10));
    SendMessage();

    // 等待网络线程结束
    networkThread.join();

    return EXIT_SUCCESS;
}
*/