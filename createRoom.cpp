//      实现创建房间功能


#include "framework.h"
#include "gameOfLife.h"
#include <winsock2.h>
#include <iphlpapi.h>
#include <ws2tcpip.h>
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")

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