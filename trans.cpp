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
ʹ��Ҫ��
Ҫ�������Ϳͻ��˴���ͬһ�������£���ͨ��ip��ַ�໥���ʣ���Զ��������Ҫʹ��������͸
������͸ʵ�֣���ʹ��Zerotier����̨��������ͬһ���������У���ʹ������IPʵ���໥��ͨ
ʹ�÷�����
    runServer(GridType& grid)���ڷ������˵Ŀ�ʼ����ÿ5s����������֤�����ߣ������ܿͻ��˵Ĳ�����������
    runClient(const char* IP,GridType& grid,json change)���ڿͻ��˵Ŀ�ʼ����ÿ5s����������֤�����ߣ������ܷ������˵�����״̬��������
    �������Ϳͻ��˵Ľ��շ��͹��ܽ�Ϊ�������߳�ʵ�֣������������̣߳����ú���ʱ��Ҫ�����̵߳���
    ��ʼ�����ʹ��send()��������ʵ�����ݷ���
*/
using namespace std; 
using json = nlohmann::json;
using INT = int;
using GridType = std::unordered_map<INT, std::unordered_map<INT, BOOL>>;
bool go=true;
bool ifSend = false;

// �� grid ת��Ϊ JSON
static json serializeGrid(const GridType& grid) {
    json j;
    for (const auto& [y, row] : grid) {
        for (const auto& [x, value] : row) {
            j[std::to_string(y)][std::to_string(x)] = value;
        }
    }
    return j;
}

// �����л� JSON �� grid
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
                std::cout << "�յ��ͻ�������: " << receivedData << std::endl;
                
                if (receivedData != "go")
                {
                    //��ӶԿͻ��������ݰ��Ĵ���

                }
                
                enet_packet_destroy(event.packet);
                waittime = 0;
                break;
            }
            case ENET_EVENT_TYPE_DISCONNECT:

                std::cout << "�ͻ��˶Ͽ����ӡ�" << std::endl;
                return;
            default:
                break;
            }
            waittime++;
        }
        if (waittime > 200)go=false; //20sδ�յ����ݰ�����Ϊ����
    }
}

void runServer(GridType& grid) {
    ENetAddress address;
    address.host = ENET_HOST_ANY;
    address.port = 12138;

    ENetHost* server = enet_host_create(&address, 1, 2, 0, 0);
    if (server == nullptr) {
        std::cerr << "��������ʼ��ʧ�ܣ�" << std::endl;
        exit(EXIT_FAILURE);
    }

    std::cout << "�����������������˿� 12138���ȴ��ͻ�������..." << std::endl;

    ENetEvent event;
    if (enet_host_service(server, &event, 50000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT) {
        std::cout << "�ͻ��������ӡ�" << std::endl;
    }
    else {
        std::cerr << "�ͻ������ӳ�ʱ��" << std::endl;
        enet_host_destroy(server);
        return;
    }

    // �ȴ��ͻ��˷��� "׼��" ָ��
    while (true) {
        if (enet_host_service(server, &event, 10000) > 0 && event.type == ENET_EVENT_TYPE_RECEIVE) {
            std::string message(reinterpret_cast<char*>(event.packet->data));
            if (message == "׼��") {
                std::cout << "�յ��ͻ��� '׼��' ָ�������ѭ����" << std::endl;
                enet_packet_destroy(event.packet);
                go = true;
                break;
            }
        }
    }

    // �����������ķ��ͺͽ����߳�
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

        std::string jsonData = change.dump(); //�û�����������
        ENetPacket* packet = enet_packet_create(jsonData.c_str(), jsonData.size() + 1, ENET_PACKET_FLAG_RELIABLE);
        enet_peer_send(peer, 0, packet);
        enet_host_flush(peer->host);
        std::cout << "�ͻ��˷��Ͳ�������" << std::endl;
        ifSend = false;
    }
}

void clientBeatSendLoop(ENetPeer* peer) {
    while (go) {
        ENetPacket* packet = enet_packet_create("go", 3, ENET_PACKET_FLAG_RELIABLE);
        enet_peer_send(peer, 0, packet);
        enet_host_flush(peer->host);
        std::cout << "�ͻ��˷�����������" << std::endl;
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
                std::cout << "�յ�����������: " << receivedData << std::endl;
                waittime = 0;
                enet_packet_destroy(event.packet);
            }
            else if (event.type == ENET_EVENT_TYPE_DISCONNECT) {
                go = false;
                std::cout << "��������Ͽ����ӡ�" << std::endl;
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
        std::cerr << "�ͻ��˳�ʼ��ʧ�ܣ�" << std::endl;
        exit(EXIT_FAILURE);
    }

    ENetAddress address;
    enet_address_set_host(&address, serverIP);
    address.port = 12138;

    ENetPeer* peer = enet_host_connect(client, &address, 2, 0);
    if (peer == nullptr) {
        std::cerr << "�޷��������ӣ�" << std::endl;
        enet_host_destroy(client);
        exit(EXIT_FAILURE);
    }

    ENetEvent event;
    if (enet_host_service(client, &event, 5000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT) {
        std::cout << "�ɹ����ӵ�������: " << serverIP << std::endl;
    }
    else {
        std::cerr << "����ʧ�ܻ�ʱ��" << std::endl;
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
    // ��ʼ��ENet
    GridType grid;
    json change;
    grid[1][1] = 1;
    if (enet_initialize() != 0) {
        std::cerr << "ENet��ʼ��ʧ�ܣ�" << std::endl;
        return EXIT_FAILURE;
    }
    atexit(enet_deinitialize);

    int mode;
    cout << "����ģʽ��1����2�ͻ��ˣ�" << endl;
    cin >> mode;
    if (mode == 1) {
        runServer(grid);
    }
    else if (mode == 2) {
        string IP;
        cout << "�������ӵ�IP��" << endl;
        cin >> IP;
        runClient(IP.c_str(),grid,change);
    }
    else {
        std::cerr << "δ֪��ģʽ����ʹ�� 1 �� 2��" << std::endl;
        return EXIT_FAILURE; 
    }

    return EXIT_SUCCESS;
}
*/
