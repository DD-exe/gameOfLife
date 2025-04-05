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
// Windows ƽ̨������Ҫ���� winsock2.h ��ͷ�ļ�
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#pragma comment(lib, "winmm.lib")

using namespace std;
using json = nlohmann::json;
using INT = int;
using GridType = std::unordered_map<INT, std::unordered_map<INT, BOOL>>;

// ȫ�ֿ��Ʊ�����ʹ��ԭ�����ͱ�֤�̰߳�ȫ��
atomic<bool> go(true);
atomic<bool> ifsend(false);

// �� grid ת��Ϊ JSON
static json serializeGrid(const GridType& grid) {
    json j;
    for (const auto& [y, row] : grid) {
        for (const auto& [x, value] : row) {
            j[to_string(y)][to_string(x)] = value;
        }
    }
    return j;
}

// �����л� JSON �� grid
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
                    cout << "�յ��ͻ�������: " << receivedData << endl;
                    if (receivedData != "go") {
                        // ��ӶԿͻ��˲������ݰ��Ĵ���
                    }
                    enet_packet_destroy(event.packet);
                    break;
                }
                case ENET_EVENT_TYPE_DISCONNECT:
                    cout << "�ͻ��˶Ͽ����ӡ�" << endl;
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
        if (idleCounter > 200) { // ����20��δ�յ����ݰ�����Ϊ����
            go = false;
        }
    }
}

BOOL runServer(GridType& grid) {
    ENetAddress address;
    address.host = ENET_HOST_ANY;
    address.port = 12138;

    ENetHost* server = enet_host_create(&address, 1, 2, 0, 0); // �˴������⣬ʲôhost.c��
                                                               // ����/lib��enet.lib���?
    if (server == nullptr) {
        cerr << "��������ʼ��ʧ�ܣ�" << endl;
        return FALSE;// exit(EXIT_FAILURE);
    }

    cout << "�����������������˿� 12138���ȴ��ͻ�������..." << endl;

    ENetEvent event;
    if (enet_host_service(server, &event, 50000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT) {
        cout << "�ͻ��������ӡ�" << endl;
    }
    else {
        cerr << "�ͻ������ӳ�ʱ��" << endl;
        enet_host_destroy(server);
        return FALSE;
    }

    // �ȴ��ͻ��˷��� "׼��" ָ��
    while (true) {
        if (enet_host_service(server, &event, 10000) > 0 && event.type == ENET_EVENT_TYPE_RECEIVE) {
            string message(reinterpret_cast<char*>(event.packet->data));
            if (message == "׼��") {
                cout << "�յ��ͻ��� '׼��' ָ�������ѭ����" << endl;
                enet_packet_destroy(event.packet);
                go = true;
                break;
            }
        }
    }

    // �����������ķ��ͺͽ����߳�
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
        string jsonData = change.dump(); // �û�����������
        ENetPacket* packet = enet_packet_create(jsonData.c_str(), jsonData.size() + 1, ENET_PACKET_FLAG_RELIABLE);
        enet_peer_send(peer, 0, packet);
        enet_host_flush(peer->host);
        cout << "�ͻ��˷��Ͳ�������" << endl;
        ifsend = false;
        this_thread::sleep_for(chrono::seconds(1)); // ��ֹæ�ȴ�
    }
}

void clientBeatSendLoop(ENetPeer* peer) {
    while (go) {
        ENetPacket* packet = enet_packet_create("go", 3, ENET_PACKET_FLAG_RELIABLE);
        enet_peer_send(peer, 0, packet);
        enet_host_flush(peer->host);
        cout << "�ͻ��˷�����������" << endl;
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
                            cerr << "����JSON���ݳ���: " << e.what() << endl;
                        }
                    }
                    cout << "�յ�����������: " << receivedData << endl;
                    enet_packet_destroy(event.packet);
                }
                else if (event.type == ENET_EVENT_TYPE_DISCONNECT) {
                    go = false;
                    cout << "��������Ͽ����ӡ�" << endl;
                    break;
                }
            } while (enet_host_service(client, &event, 0) > 0);
        }
        else {
            idleCounter++;
        }
        if (idleCounter > 200) { // ����20����������Ϊ����
            go = false;
        }
    }
}

BOOL runClient(const char* serverIP, GridType& grid, json& change) {
    ENetHost* client = enet_host_create(nullptr, 1, 2, 0, 0);
    if (client == nullptr) {
        cerr << "�ͻ��˳�ʼ��ʧ�ܣ�" << endl;
        return FALSE;// exit(EXIT_FAILURE);
    }

    ENetAddress address;
    enet_address_set_host(&address, serverIP);
    address.port = 12138;

    ENetPeer* peer = enet_host_connect(client, &address, 2, 0);
    if (peer == nullptr) {
        cerr << "�޷��������ӣ�" << endl;
        enet_host_destroy(client);
        return FALSE; // exit(EXIT_FAILURE);
    }

    ENetEvent event;
    if (enet_host_service(client, &event, 5000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT) {
        cout << "�ɹ����ӵ�������: " << serverIP << endl;
    }
    else {
        cerr << "����ʧ�ܻ�ʱ��" << endl;
        enet_peer_reset(peer);
        enet_host_destroy(client);
        return FALSE; // exit(EXIT_FAILURE);
    }

    // ���� "׼��" ָ��
    const char* readyMsg = "׼��";
    ENetPacket* packet = enet_packet_create(readyMsg, std::strlen(readyMsg) + 1, ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send(peer, 0, packet);
    enet_host_flush(client);
    go = true;

    // �������ͺͽ����߳�
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
    // ��ʼ��ENet
    GridType grid;
    json change;
    change["test"] = "111";
    grid[1][1] = 1;

    if (enet_initialize() != 0) {
        cerr << "ENet��ʼ��ʧ�ܣ�" << endl;
        return EXIT_FAILURE;
    }
    atexit(enet_deinitialize);

    int mode;
    cout << "����ģʽ��1���� 2�ͻ��ˣ�" << endl;
    cin >> mode;

    thread networkThread;
    if (mode == 1) {
        // ������ʱ��ʹ�� std::ref
        networkThread = thread(runServer, ref(grid));
    }
    else if (mode == 2) {
        string IP;
        cout << "�������ӵ�IP��" << endl;
        cin >> IP;
        networkThread = thread(runClient, IP.c_str(), ref(grid), ref(change));
    }
    else {
        cerr << "δ֪��ģʽ����ʹ�� 1 �� 2��" << endl;
        return EXIT_FAILURE;
    }

    // ģ�����߳���10��󴥷����Ͳ���
    this_thread::sleep_for(chrono::seconds(10));
    SendMessage();

    // �ȴ������߳̽���
    networkThread.join();

    return EXIT_SUCCESS;
}
*/