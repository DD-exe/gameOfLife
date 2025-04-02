//      ʵ�ִ������书��


#include "framework.h"
#include "gameOfLife.h"
#include <winsock2.h>
#include <iphlpapi.h>
#include <ws2tcpip.h>
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")

// ��ȡ����IPv4��ַ�ĺ���
bool GetLocalIPAddress(wchar_t* ipBuffer, int bufferSize)
{
    PIP_ADAPTER_ADDRESSES pAddresses = NULL;
    ULONG outBufLen = 0;
    DWORD dwRetVal = 0;

    // ��һ�ε��û�ȡ���軺������С
    GetAdaptersAddresses(AF_INET, GAA_FLAG_INCLUDE_PREFIX, NULL, pAddresses, &outBufLen);
    pAddresses = (PIP_ADAPTER_ADDRESSES)malloc(outBufLen);

    // �ڶ��ε���ʵ�ʻ�ȡ��������Ϣ
    dwRetVal = GetAdaptersAddresses(AF_INET, GAA_FLAG_INCLUDE_PREFIX, NULL, pAddresses, &outBufLen);

    if (dwRetVal == NO_ERROR) {
        PIP_ADAPTER_ADDRESSES pCurrAddresses = pAddresses;
        while (pCurrAddresses) {
            // �����ػ��ӿ�
            if (pCurrAddresses->IfType != IF_TYPE_SOFTWARE_LOOPBACK) {
                PIP_ADAPTER_UNICAST_ADDRESS pUnicast = pCurrAddresses->FirstUnicastAddress;
                while (pUnicast) {
                    if (pUnicast->Address.lpSockaddr->sa_family == AF_INET) {
                        sockaddr_in* sa_in = (sockaddr_in*)pUnicast->Address.lpSockaddr;
                        InetNtop(AF_INET, &(sa_in->sin_addr), ipBuffer, bufferSize);
                        free(pAddresses);
                        return true;
                    }
                    pUnicast = pUnicast->Next;
                }
            }
            pCurrAddresses = pCurrAddresses->Next;
        }
    }

    // �����ȡʧ��
    wcscpy_s(ipBuffer, bufferSize, L"�޷���ȡIP");
    if (pAddresses) free(pAddresses);
    return false;
}

INT_PTR CALLBACK CreateRoomProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static wchar_t roomName[100] = { 0 };
    static wchar_t playerName[100] = { 0 };

    switch (message)
    {
    case WM_INITDIALOG:
    {
        // ��ʼ��Winsock
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData);

        // ��ȡ����ʾ����IP
        wchar_t ipBuffer[46] = { 0 }; // IPv4��󳤶�
        if (GetLocalIPAddress(ipBuffer, sizeof(ipBuffer) / sizeof(wchar_t))) {
            SetDlgItemText(hDlg, IDC_LOCALIP, ipBuffer);
            SetDlgItemText(hDlg, IDC_PLAYERNUMBER, L"0");
        }
        else {
            SetDlgItemText(hDlg, IDC_LOCALIP, L"��ȡIPʧ��");
            SetDlgItemText(hDlg, IDC_PLAYERNUMBER, L"0");
        }

        return (INT_PTR)TRUE;
    }

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:  // "ȷ��"��ť

            // �������ʵ�ʴ���������߼�
            // ���磺��������ģ�鴴������

            // ��ʾ���������ʵ����Ŀ�п���ȥ����
            wchar_t msg[256];
            swprintf(msg, 256, L"���䴴���ɹ���");
            MessageBox(hDlg, msg, L"�����ɹ�", MB_OK);

            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
            break;
        case IDCANCEL:  // "ȡ��"��ť
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    case WM_DESTROY:
        WSACleanup(); // ����Winsock
        EndDialog(hDlg, LOWORD(wParam));
        return (INT_PTR)TRUE;

    }
    return (INT_PTR)FALSE;
}