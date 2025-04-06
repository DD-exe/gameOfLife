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

struct shortVsoData {
    int att[2];
    int def[2];
    int muv[2];
    int suv[2];
    // grid Ϊ��СΪ2�����飬ÿ��Ԫ��Ϊһ��Ƕ�׵� unordered_map
    std::unordered_map<int, std::unordered_map<int, bool>> grid[2];
};

struct serverVsoData {
    int att[2];
    int def[2];
    int muv[2];
    int suv[2];
};

// ���л�����
json serialize(const serverVsoData& data) {
    nlohmann::json j;
    j["att"] = { data.att[0], data.att[1] };
    j["def"] = { data.def[0], data.def[1] };
    j["muv"] = { data.muv[0], data.muv[1] };
    j["suv"] = { data.suv[0], data.suv[1] };
    return j;
}

// �����л�����
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

    // ���� att ����
    result.att[0] = original.att[0];
    result.att[1] = original.att[1];

    // ���� def ����
    result.def[0] = original.def[0];
    result.def[1] = original.def[1];

    // ���� muv ����
    result.muv[0] = original.muv[0];
    result.muv[1] = original.muv[1];

    // ���� suv ����
    result.suv[0] = original.suv[0];
    result.suv[1] = original.suv[1];

    // ���� grid ����

    return result;
}

shortVsoData extractShortVsoData(const vsoData& original) {
    shortVsoData result;

    // ���� att ����
    result.att[0] = original.att[0];
    result.att[1] = original.att[1];

    // ���� def ����
    result.def[0] = original.def[0];
    result.def[1] = original.def[1];

    // ���� muv ����
    result.muv[0] = original.muv[0];
    result.muv[1] = original.muv[1];

    // ���� suv ����
    result.suv[0] = original.suv[0];
    result.suv[1] = original.suv[1];

    // ���� grid ����
    for (int i = 0; i < 2; ++i) {
        result.grid[i].insert(original.grid[i].begin(), original.grid[i].end());
    }

    return result;
}

// ���л��������� vsoData ����ת��Ϊ JSON ����
json serializeVsoData(const shortVsoData& data) {
    json j;
    // ���л��̶�����
    j["att"] = { data.att[0], data.att[1] };
    j["def"] = { data.def[0], data.def[1] };
    j["muv"] = { data.muv[0], data.muv[1] };
    j["suv"] = { data.suv[0], data.suv[1] };

    // ���л� grid ���飨����Ϊ2��
    j["grid"] = json::array();
    for (int i = 0; i < 2; ++i) {
        json gridJson = json::object();
        // ������� map
        for (const auto& outerPair : data.grid[i]) {
            int key1 = outerPair.first;
            json innerJson = json::object();
            // �����ڲ� map
            for (const auto& innerPair : outerPair.second) {
                int key2 = innerPair.first;
                bool value = innerPair.second;
                // JSON �� key ����Ϊ�ַ�������������� std::to_string ת��
                innerJson[std::to_string(key2)] = value;
            }
            gridJson[std::to_string(key1)] = innerJson;
        }
        j["grid"].push_back(gridJson);
    }
    return j;
}

// �����л��������� JSON ����ԭ vsoData ����
shortVsoData deserializeVsoData(const json& j) {
    shortVsoData data;
    // ��ԭ�̶�����
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

    // ��ԭ grid ����
    if (j.contains("grid") && j["grid"].is_array() && j["grid"].size() == 2) {
        for (int i = 0; i < 2; ++i) {
            json gridJson = j["grid"][i];
            // ����������
            for (auto it = gridJson.begin(); it != gridJson.end(); ++it) {
                int key1 = std::stoi(it.key());
                // �����ڲ����
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
                    cout << "�յ��ͻ�������: " << receivedData << endl;
                    if (receivedData != "go") {
                        // ͬ���ͻ��˵Ĳ�������
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

BOOL runServer(vsoData& data) {
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

                            // ���� def ����
                            mainData.def[0] = shortData.def[0];
                            mainData.def[1] = shortData.def[1];

                            // ���� muv ����
                            mainData.muv[0] = shortData.muv[0];
                            mainData.muv[1] = shortData.muv[1];

                            // ���� suv ����
                            mainData.suv[0] = shortData.suv[0];
                            mainData.suv[1] = shortData.suv[1];

                            // ���� grid ����
                            for (int i = 0; i < 2; ++i) {
                                mainData.grid[i].insert(shortData.grid[i].begin(), shortData.grid[i].end());
                            }
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

BOOL runClient(vsoData& data) {
    ENetHost* client = enet_host_create(nullptr, 1, 2, 0, 0);
    if (client == nullptr) {
        cerr << "�ͻ��˳�ʼ��ʧ�ܣ�" << endl;
        return FALSE;// exit(EXIT_FAILURE);
    }
    ENetAddress address;
    enet_address_set_host(&address, wc2s(data.targetIP).c_str());
    address.port = 12138;

    ENetPeer* peer = enet_host_connect(client, &address, 2, 0);
    if (peer == nullptr) {
        cerr << "�޷��������ӣ�" << endl;
        enet_host_destroy(client);
        return FALSE; // exit(EXIT_FAILURE);
    }

    ENetEvent event;
    if (enet_host_service(client, &event, 5000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT) {
        cout << "�ɹ����ӵ�������: " << data.targetIP << endl;
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