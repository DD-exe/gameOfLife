#include "framework.h"
#include "gameOfLife.h"

// 主对话框处理函数
INT_PTR CALLBACK VSOnline(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);

    switch (message)
    {
    case WM_INITDIALOG:
        // 初始化代码（例如设置图标、默认控件状态等）
        return (INT_PTR)TRUE;

    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);

        switch (wmId)
        {
        case IDvsOnlineCREATE:  // "创建房间"按钮
            DialogBox(GetModuleHandle(NULL),MAKEINTRESOURCE(IDD_CREATEROOM),hDlg,CreateRoomProc);
            break;

        case IDvsOnlineJOIN:   // "加入房间"按钮
            DialogBox(GetModuleHandle(NULL),MAKEINTRESOURCE(IDD_JOINROOM),hDlg,JoinRoomProc);
            break;

        case IDCANCEL:         // 关闭对话框
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
    }
    break;
    }
    return (INT_PTR)FALSE;
}

