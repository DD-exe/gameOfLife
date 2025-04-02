//      实现创建房间功能


#include "framework.h"
#include "gameOfLife.h"
#include <winsock2.h>
#include <iphlpapi.h>
#include <ws2tcpip.h>
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")

// 获取本机IPv4地址的函数
bool GetLocalIPAddress(wchar_t* ipBuffer, int bufferSize)
{
    PIP_ADAPTER_ADDRESSES pAddresses = NULL;
    ULONG outBufLen = 0;
    DWORD dwRetVal = 0;

    // 第一次调用获取所需缓冲区大小
    GetAdaptersAddresses(AF_INET, GAA_FLAG_INCLUDE_PREFIX, NULL, pAddresses, &outBufLen);
    pAddresses = (PIP_ADAPTER_ADDRESSES)malloc(outBufLen);

    // 第二次调用实际获取适配器信息
    dwRetVal = GetAdaptersAddresses(AF_INET, GAA_FLAG_INCLUDE_PREFIX, NULL, pAddresses, &outBufLen);

    if (dwRetVal == NO_ERROR) {
        PIP_ADAPTER_ADDRESSES pCurrAddresses = pAddresses;
        while (pCurrAddresses) {
            // 跳过回环接口
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

    // 如果获取失败
    wcscpy_s(ipBuffer, bufferSize, L"无法获取IP");
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
        // 初始化Winsock
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData);

        // 获取并显示本机IP
        wchar_t ipBuffer[46] = { 0 }; // IPv4最大长度
        if (GetLocalIPAddress(ipBuffer, sizeof(ipBuffer) / sizeof(wchar_t))) {
            SetDlgItemText(hDlg, IDC_LOCALIP, ipBuffer);
            SetDlgItemText(hDlg, IDC_PLAYERNUMBER, L"0");
        }
        else {
            SetDlgItemText(hDlg, IDC_LOCALIP, L"获取IP失败");
            SetDlgItemText(hDlg, IDC_PLAYERNUMBER, L"0");
        }

        return (INT_PTR)TRUE;
    }

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:  // "确定"按钮

            // 这里添加实际创建房间的逻辑
            // 例如：调用网络模块创建房间

            // 显示创建结果（实际项目中可以去掉）
            wchar_t msg[256];
            swprintf(msg, 256, L"房间创建成功！");
            MessageBox(hDlg, msg, L"创建成功", MB_OK);

            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
            break;
        case IDCANCEL:  // "取消"按钮
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    case WM_DESTROY:
        WSACleanup(); // 清理Winsock
        EndDialog(hDlg, LOWORD(wParam));
        return (INT_PTR)TRUE;

    }
    return (INT_PTR)FALSE;
}