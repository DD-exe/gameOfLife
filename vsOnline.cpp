#include "framework.h"
#include "gameOfLife.h"
INT_PTR CALLBACK VSOnline(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        INT wmId = LOWORD(wParam);
        switch (wmId)
        {
        case IDvsOnlineCREATE:
        {
            MessageBox(hDlg, L"�򿪷��䴴��ҳ��", L"��Ϣ", MB_OK);
        }
        break;
        case IDvsOnlineJOIN:
        {
            MessageBox(hDlg, L"�򿪷������ҳ��", L"��Ϣ", MB_OK);
        }
        break;
        case IDCANCEL:
            EndDialog(hDlg, LOWORD(wParam));
        }
        break;
        return (INT_PTR)TRUE;
    }
    return (INT_PTR)FALSE;
}