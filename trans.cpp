#include "framework.h"
#include <iostream>
#include <nlohmann/json.hpp>
#include <enet/enet.h>
#include <cstring>
#include <cstdlib>
#include <chrono>
#include <atomic>
#include <stdexcept>
// Windows ƽ̨������Ҫ���� winsock2.h ��ͷ�ļ�
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#pragma comment(lib, "winmm.lib")
#include "resource.h"

using namespace std;
using json = nlohmann::json;
using INT = int;
using GridType = std::unordered_map<INT, std::unordered_map<INT, BOOL>>;

// ȫ�ֿ��Ʊ�����ʹ��ԭ�����ͱ�֤�̰߳�ȫ��
atomic<bool> go(true);
atomic<bool> ifServerSend(false);
atomic<bool> ifClientSend(false);
//���л�vsoData����
static json serializeVsoData(const vsoData& data) {
    nlohmann::json j;

    // ���л� grid[2]
    j["grid"] = nlohmann::json::array();
    for (int i = 1; i >= 0; --i) {
        nlohmann::json gridJson;
        for (const auto& [x, innerMap] : data.grid[i]) {
            nlohmann::json innerJson;
            for (const auto& [y, value] : innerMap) {
                innerJson[std::to_string(y)] = value;
            }
            gridJson[std::to_string(x)] = innerJson;
        }
        j["grid"].push_back(gridJson);
    }

    // ���л����������Ա
    j["score"] = data.score[0] ;
    j["att"] =data.att[0] ;
    j["def"] = data.def[0] ;
    j["muv"] =  data.muv[0] ;
    j["suv"] =  data.suv[0] ;
    return j;
}

// �����л�vsoData����
void deserializeVsoData(vsoData& data, nlohmann::json j) {

    // �����л� grid[2]
    for (int i = 0; i < 2; ++i) {
        if (j.contains("grid") && j["grid"].size() > i) {
            for (auto& [xStr, innerJson] : j["grid"][i].items()) {
                INT x = std::stoi(xStr);
                for (auto& [yStr, value] : innerJson.items()) {
                    INT y = std::stoi(yStr);
                    data.grid[i][x][y] = value;
                }
            }
        }
    }

    // �����л����������Ա
    data.score[1] = j["score"];
    data.att[1] = j["att"];
    data.def[1] = j["def"];
    data.muv[1] = j["muv"];
    data.suv[1] = j["suv"];
}

void serverSendMessage() {
    ifServerSend = true;
}

void clientSendMessage() {
    ifClientSend = true;
}

void serverSendLoop(HWND hDlg, ENetPeer* peer, vsoData& mainData) {
    while (go) {
        if (!mainData.send) {
            this_thread::sleep_for(chrono::seconds(2));
            continue;
        }
        json data0 = serializeVsoData(mainData);
        json data;
        data["handle"] = reinterpret_cast<std::uintptr_t>(hDlg);
        data["data"] = data0;
        string jsonData = data.dump();
        ENetPacket* packet = enet_packet_create(jsonData.c_str(), jsonData.size() + 1, ENET_PACKET_FLAG_RELIABLE);
        enet_peer_send(peer, 0, packet);
        enet_host_flush(peer->host);
        mainData.send = false;
        EnableWindow(GetDlgItem(hDlg, ID_START), FALSE);
        EnableWindow(GetDlgItem(hDlg, ID_STOP), FALSE);
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

void serverReceiveLoop(HWND hDlg,ENetHost* server, vsoData& mainData) {
    ENetEvent event;
    int idleCounter = 0;
    while (go) {
        int eventCount = enet_host_service(server, &event, 100);
        if (eventCount > 0) {
            idleCounter = 0;
            do {
                switch (event.type) {
                case ENET_EVENT_TYPE_RECEIVE: {
                    if (!event.packet || event.packet->dataLength == 0) {
                        printf("Received invalid packet\n");
                        return;
                    }
                    try {
                        // ������ת��Ϊ�ַ�����ע�⣺packet->data���ܲ���\0��β��
                        std::string jsonStr((const char*)event.packet->data, event.packet->dataLength);

                        // ����JSON
                        nlohmann::json jsonData = nlohmann::json::parse(jsonStr);
                        HWND h = reinterpret_cast<HWND>(jsonData["handle"].get<std::uintptr_t>());
                        if (h != hDlg)
                        {
                            //ɾ�����в���
                            mainData.grid[0].clear();
                            mainData.grid[1].clear();
                            deserializeVsoData(mainData, jsonData["data"]);
                        }
                        PostMessage(hDlg, WM_RECEIVE, 0, 0);
                    }
                    catch (const std::exception& e) {
                        printf("JSON parse error: %s\n", e.what());
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

void runServer(vsoData& data, HWND hDlg) {
    if (enet_initialize())exit(EXIT_FAILURE);
    ENetAddress address;
    address.host = ENET_HOST_ANY;
    address.port = 12138;
    ENetHost* server = enet_host_create(&address, 1, 2, 0, 0);
    if (server == nullptr) {
        cerr << "��������ʼ��ʧ�ܣ�" << endl;
        exit(EXIT_FAILURE);
    }

    cout << "�����������������˿� 12138���ȴ��ͻ�������..." << endl;

    ENetEvent event;
    if (enet_host_service(server, &event, 50000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT) {
        cout << "�ͻ��������ӡ�" << endl;
    }
    else {
        cerr << "�ͻ������ӳ�ʱ��" << endl;
        enet_host_destroy(server);
        exit(EXIT_FAILURE);
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
    thread sendThread(serverSendLoop,hDlg, event.peer, ref(data));
    thread receiveThread(serverReceiveLoop, hDlg, server, ref(data));
    data.ifServer = TRUE;
    PostMessage(hDlg, WM_SERVER_WAITING, 0, 0);
    beatThread.join();
    sendThread.join();
    receiveThread.join();

    enet_host_destroy(server);
}

void clientSendLoop(HWND hDlg, ENetPeer* peer, vsoData& mainData) {
    while (go) {
        if (!mainData.send) {
            this_thread::sleep_for(chrono::seconds(3));
            continue;
        }
        json data0 = serializeVsoData(mainData);
        json data;
        data["handle"] = reinterpret_cast<std::uintptr_t>(hDlg);
        data["data"] = data0;
        string jsonData = data.dump(); // �û�����������
        ENetPacket* packet = enet_packet_create(jsonData.c_str(), jsonData.size() + 1, ENET_PACKET_FLAG_RELIABLE);
        enet_peer_send(peer, 0, packet);
        enet_host_flush(peer->host);
        cout << "�ͻ��˷��Ͳ�������" << endl;
        mainData.send = false;
        EnableWindow(GetDlgItem(hDlg, ID_START), FALSE);
        EnableWindow(GetDlgItem(hDlg, ID_STOP), FALSE);
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

void clientReceiveLoop(HWND hDlg,ENetHost* client, vsoData& mainData) {
    ENetEvent event;
    int idleCounter = 0;
    while (go) {
        int eventCount = enet_host_service(client, &event, 100);
        if (eventCount > 0) {
            idleCounter = 0;
            do {
                if (event.type == ENET_EVENT_TYPE_RECEIVE) {
                    if (!event.packet || event.packet->dataLength == 0) {
                        printf("Received invalid packet\n");
                        return;
                    }
                    try {
                        // ������ת��Ϊ�ַ�����ע�⣺packet->data���ܲ���\0��β��
                        std::string jsonStr((const char*)event.packet->data, event.packet->dataLength);

                        // ����JSON
                        nlohmann::json jsonData = nlohmann::json::parse(jsonStr);

                        HWND h = reinterpret_cast<HWND>(jsonData["handle"].get<std::uintptr_t>());
                        if (h != hDlg)
                        {
                            //ɾ�����в���
                            mainData.grid[0].clear();
                            mainData.grid[1].clear();
                            deserializeVsoData(mainData, jsonData["data"]);
                        }
                        PostMessage(hDlg, WM_RECEIVE, 0, 0);
                    }
                    catch (const std::exception& e) {
                        printf("JSON parse error: %s\n", e.what());
                    }
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

void runClient(vsoData& data, HWND hDlg) {
    if (enet_initialize())exit(EXIT_FAILURE);
    ENetHost* client = enet_host_create(nullptr, 1, 2, 0, 0);
    if (client == nullptr) {
        cerr << "�ͻ��˳�ʼ��ʧ�ܣ�" << endl;
        exit(EXIT_FAILURE);
    }
    ENetAddress address;
    enet_address_set_host(&address, wc2s(data.targetIP).c_str());
    address.port = 12138;

    ENetPeer* peer = enet_host_connect(client, &address, 2, 0);
    if (peer == nullptr) {
        cerr << "�޷��������ӣ�" << endl;
        enet_host_destroy(client);
        exit(EXIT_FAILURE);
    }

    ENetEvent event;
    if (enet_host_service(client, &event, 5000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT) {
        cout << "�ɹ����ӵ�������: " << data.targetIP << endl;
    }
    else {
        cerr << "����ʧ�ܻ�ʱ��" << endl;
        enet_peer_reset(peer);
        enet_host_destroy(client);
        exit(EXIT_FAILURE);
    }

    // ���� "׼��" ָ��
    const char* readyMsg = "׼��";
    ENetPacket* packet = enet_packet_create(readyMsg, std::strlen(readyMsg) + 1, ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send(peer, 0, packet);
    enet_host_flush(client);
    go = true;

    // �������ͺͽ����߳�
    thread beatThread(clientBeatSendLoop, peer);
    thread sendThread(clientSendLoop, hDlg, peer, ref(data));
    thread receiveThread(clientReceiveLoop, hDlg, client, ref(data));
    data.ifClient = TRUE;
    PostMessage(hDlg, WM_CLIENT_WAITING, 0, 0);

    beatThread.join();
    sendThread.join();
    receiveThread.join();

    enet_host_destroy(client);
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