//实现加入房间窗口
#include "framework.h"
#include "gameOfLife.h"

INT_PTR CALLBACK JoinRoomProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static wchar_t roomIP[100] = { 0 };
    static wchar_t playerName[100] = { 0 };

    switch (message)
    {
    case WM_INITDIALOG:
        // 设置默认IP和玩家名
        SetDlgItemText(hDlg, IDC_ROOMIP, L"127.0.0.1");
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:  // "确定"按钮
            // 获取用户输入
            GetDlgItemText(hDlg, IDC_ROOMIP, roomIP, 100);

            // 这里添加实际加入房间的逻辑
            // 例如：调用网络模块连接指定IP

            // 显示加入结果（实际项目中可以去掉）
            wchar_t msg[256];
            swprintf(msg, 256, L"正在加入房间...\nIP地址: %s\n", roomIP);
            MessageBox(hDlg, msg, L"加入房间", MB_OK);

            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;

        case IDCANCEL:  // "取消"按钮
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}