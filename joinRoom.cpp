//ʵ�ּ��뷿�䴰��
#include "framework.h"
#include "gameOfLife.h"

INT_PTR CALLBACK JoinRoomProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static wchar_t roomIP[100] = { 0 };
    static wchar_t playerName[100] = { 0 };

    switch (message)
    {
    case WM_INITDIALOG:
        // ����Ĭ��IP�������
        SetDlgItemText(hDlg, IDC_ROOMIP, L"127.0.0.1");
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:  // "ȷ��"��ť
            // ��ȡ�û�����
            GetDlgItemText(hDlg, IDC_ROOMIP, roomIP, 100);

            // �������ʵ�ʼ��뷿����߼�
            // ���磺��������ģ������ָ��IP

            // ��ʾ��������ʵ����Ŀ�п���ȥ����
            wchar_t msg[256];
            swprintf(msg, 256, L"���ڼ��뷿��...\nIP��ַ: %s\n", roomIP);
            MessageBox(hDlg, msg, L"���뷿��", MB_OK);

            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;

        case IDCANCEL:  // "ȡ��"��ť
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}