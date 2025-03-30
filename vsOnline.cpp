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
            MessageBox(hDlg, L"打开房间创建页面", L"信息", MB_OK);
        }
        break;
        case IDvsOnlineJOIN:
        {
            MessageBox(hDlg, L"打开房间加入页面", L"信息", MB_OK);
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