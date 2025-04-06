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

struct shortVsoData {
    int att[2];
    int def[2];
    int muv[2];
    int suv[2];
    // grid 为大小为2的数组，每个元素为一个嵌套的 unordered_map
    std::unordered_map<int, std::unordered_map<int, bool>> grid[2];
};

struct serverVsoData {
    int att[2];
    int def[2];
    int muv[2];
    int suv[2];
};

// 序列化函数
json serialize(const serverVsoData& data) {
    nlohmann::json j;
    j["att"] = { data.att[0], data.att[1] };
    j["def"] = { data.def[0], data.def[1] };
    j["muv"] = { data.muv[0], data.muv[1] };
    j["suv"] = { data.suv[0], data.suv[1] };
    return j;
}

// 反序列化函数
serverVsoData deserialize(const std::string& jsonStr) {
    serverVsoData data;
    nlohmann::json j = nlohmann::json::parse(jsonStr);
    for (int i = 0; i < 2; ++i) {
        data.att[i] = j["att"][i];
        data.def[i] = j["def"][i];
        data.muv[i] = j["muv"][i];
        data.suv[i] = j["suv"][i];
    }
    return data;
}

serverVsoData extractServerVsoData(const vsoData& original) {
    serverVsoData result;

    // 复制 att 数组
    result.att[0] = original.att[0];
    result.att[1] = original.att[1];

    // 复制 def 数组
    result.def[0] = original.def[0];
    result.def[1] = original.def[1];

    // 复制 muv 数组
    result.muv[0] = original.muv[0];
    result.muv[1] = original.muv[1];

    // 复制 suv 数组
    result.suv[0] = original.suv[0];
    result.suv[1] = original.suv[1];

    // 复制 grid 数组

    return result;
}

shortVsoData extractShortVsoData(const vsoData& original) {
    shortVsoData result;

    // 复制 att 数组
    result.att[0] = original.att[0];
    result.att[1] = original.att[1];

    // 复制 def 数组
    result.def[0] = original.def[0];
    result.def[1] = original.def[1];

    // 复制 muv 数组
    result.muv[0] = original.muv[0];
    result.muv[1] = original.muv[1];

    // 复制 suv 数组
    result.suv[0] = original.suv[0];
    result.suv[1] = original.suv[1];

    // 复制 grid 数组
    for (int i = 0; i < 2; ++i) {
        result.grid[i].insert(original.grid[i].begin(), original.grid[i].end());
    }

    return result;
}

// 序列化函数，将 vsoData 对象转换为 JSON 对象
json serializeVsoData(const shortVsoData& data) {
    json j;
    // 序列化固定数组
    j["att"] = { data.att[0], data.att[1] };
    j["def"] = { data.def[0], data.def[1] };
    j["muv"] = { data.muv[0], data.muv[1] };
    j["suv"] = { data.suv[0], data.suv[1] };

    // 序列化 grid 数组（长度为2）
    j["grid"] = json::array();
    for (int i = 0; i < 2; ++i) {
        json gridJson = json::object();
        // 遍历外层 map
        for (const auto& outerPair : data.grid[i]) {
            int key1 = outerPair.first;
            json innerJson = json::object();
            // 遍历内层 map
            for (const auto& innerPair : outerPair.second) {
                int key2 = innerPair.first;
                bool value = innerPair.second;
                // JSON 的 key 必须为字符串，因此这里用 std::to_string 转换
                innerJson[std::to_string(key2)] = value;
            }
            gridJson[std::to_string(key1)] = innerJson;
        }
        j["grid"].push_back(gridJson);
    }
    return j;
}

// 反序列化函数，从 JSON 对象还原 vsoData 对象
shortVsoData deserializeVsoData(const json& j) {
    shortVsoData data;
    // 还原固定数组
    if (j.contains("att") && j["att"].is_array() && j["att"].size() == 2) {
        data.att[0] = j["att"][0].get<int>();
        data.att[1] = j["att"][1].get<int>();
    }
    if (j.contains("def") && j["def"].is_array() && j["def"].size() == 2) {
        data.def[0] = j["def"][0].get<int>();
        data.def[1] = j["def"][1].get<int>();
    }
    if (j.contains("muv") && j["muv"].is_array() && j["muv"].size() == 2) {
        data.muv[0] = j["muv"][0].get<int>();
        data.muv[1] = j["muv"][1].get<int>();
    }
    if (j.contains("suv") && j["suv"].is_array() && j["suv"].size() == 2) {
        data.suv[0] = j["suv"][0].get<int>();
        data.suv[1] = j["suv"][1].get<int>();
    }

    // 还原 grid 数组
    if (j.contains("grid") && j["grid"].is_array() && j["grid"].size() == 2) {
        for (int i = 0; i < 2; ++i) {
            json gridJson = j["grid"][i];
            // 遍历外层对象
            for (auto it = gridJson.begin(); it != gridJson.end(); ++it) {
                int key1 = std::stoi(it.key());
                // 遍历内层对象
                for (auto innerIt = it.value().begin(); innerIt != it.value().end(); ++innerIt) {
                    int key2 = std::stoi(innerIt.key());
                    bool value = innerIt.value().get<bool>();
                    data.grid[i][key1][key2] = value;
                }
            }
        }
    }
    return data;
}

void mySendMessage() {
    ifsend = true;
}

void serverSendLoop(ENetPeer* peer, vsoData& mainData) {
    while (go) {
        if (!ifsend) {
            this_thread::sleep_for(chrono::seconds(1));
            continue;
        }
        json data = serializeVsoData(extractShortVsoData(mainData));
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

void serverReceiveLoop(ENetHost* server, vsoData& mainData) {
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
                        // 同步客户端的操作数据
                        serverVsoData j=deserialize(receivedData);

                        mainData.att[0] = j.att[0];
                        mainData.att[1] = j.att[1];
                        mainData.def[0] = j.def[0];
                        mainData.def[1] = j.def[1];
                        mainData.muv[0] = j.muv[0];
                        mainData.muv[1] = j.muv[1];
                        mainData.suv[0] = j.suv[0];
                        mainData.suv[1] = j.suv[1];

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

BOOL runServer(vsoData& data) {
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
    thread sendThread(serverSendLoop, event.peer, ref(data));
    thread receiveThread(serverReceiveLoop, server, ref(data));

    beatThread.join();
    sendThread.join();
    receiveThread.join();

    enet_host_destroy(server);
    return TRUE;
}

void clientSendLoop(ENetPeer* peer, vsoData& mainData) {
    while (go) {
        if (!ifsend) {
            this_thread::sleep_for(chrono::seconds(1));
            continue;
        }
        json change = serialize(extractServerVsoData(mainData));
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

void clientReceiveLoop(ENetHost* client, vsoData& mainData) {
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
                            shortVsoData shortData = deserializeVsoData(newdata);
                            mainData.att[0] = shortData.att[0];
                            mainData.att[1] = shortData.att[1];

                            // 复制 def 数组
                            mainData.def[0] = shortData.def[0];
                            mainData.def[1] = shortData.def[1];

                            // 复制 muv 数组
                            mainData.muv[0] = shortData.muv[0];
                            mainData.muv[1] = shortData.muv[1];

                            // 复制 suv 数组
                            mainData.suv[0] = shortData.suv[0];
                            mainData.suv[1] = shortData.suv[1];

                            // 复制 grid 数组
                            for (int i = 0; i < 2; ++i) {
                                mainData.grid[i].insert(shortData.grid[i].begin(), shortData.grid[i].end());
                            }
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

BOOL runClient(vsoData& data) {
    ENetHost* client = enet_host_create(nullptr, 1, 2, 0, 0);
    if (client == nullptr) {
        cerr << "客户端初始化失败！" << endl;
        return FALSE;// exit(EXIT_FAILURE);
    }
    ENetAddress address;
    enet_address_set_host(&address, wc2s(data.targetIP).c_str());
    address.port = 12138;

    ENetPeer* peer = enet_host_connect(client, &address, 2, 0);
    if (peer == nullptr) {
        cerr << "无法创建连接！" << endl;
        enet_host_destroy(client);
        return FALSE; // exit(EXIT_FAILURE);
    }

    ENetEvent event;
    if (enet_host_service(client, &event, 5000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT) {
        cout << "成功连接到服务器: " << data.targetIP << endl;
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
    thread sendThread(clientSendLoop, peer, ref(data));
    thread receiveThread(clientReceiveLoop, client, ref(data));

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