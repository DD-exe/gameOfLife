//      ʵ�ִ������书��


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