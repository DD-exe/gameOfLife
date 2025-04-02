#include "framework.h"
#include "gameOfLife.h"

// ���Ի�������
INT_PTR CALLBACK VSOnline(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);

    switch (message)
    {
    case WM_INITDIALOG:
        // ��ʼ�����루��������ͼ�ꡢĬ�Ͽؼ�״̬�ȣ�
        return (INT_PTR)TRUE;

    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);

        switch (wmId)
        {
        case IDvsOnlineCREATE:  // "��������"��ť
            DialogBox(GetModuleHandle(NULL),MAKEINTRESOURCE(IDD_CREATEROOM),hDlg,CreateRoomProc);
            break;

        case IDvsOnlineJOIN:   // "���뷿��"��ť
            DialogBox(GetModuleHandle(NULL),MAKEINTRESOURCE(IDD_JOINROOM),hDlg,JoinRoomProc);
            break;

        case IDCANCEL:         // �رնԻ���
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
    }
    break;
    }
    return (INT_PTR)FALSE;
}

