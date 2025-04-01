/*
    用于实现“选色”对话框消息处理
*/
#include "framework.h"
#include "gameOfLife.h"

INT_PTR CALLBACK color(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    coData* data = (coData*)GetWindowLongPtr(hDlg, GWLP_USERDATA);
    switch (message)
    {
    case WM_INITDIALOG:
    {
        coData* data = new coData();
        LPCREATESTRUCT lData = (LPCREATESTRUCT)lParam;
        // data->hParent = (HWND)lData->lpCreateParams;
        data->r = 0; data->g = 0; data->b = 0;
        data->ifMouseDown = FALSE;
        SetWindowLongPtr(hDlg, GWLP_USERDATA, (LONG_PTR)data);
        return (INT_PTR)TRUE;
    }
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        switch (wmId)
        {
        case IDOK:
        {
            EndDialog(hDlg, RGB(data->r,data->g,data->b)); // 返回选中的颜色
            return (INT_PTR)TRUE;
        }
        case IDCANCEL:
            EndDialog(hDlg, NULL);
            return (INT_PTR)TRUE;
        }
        break;
    }
    case WM_PAINT: 
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hDlg, &ps);
        if (data->ifMouseDown) {
            INT x = 1;
        }
        // 直接绘制颜色图
        for (int y = 0; y < 255; y++) {
            for (int x = 0; x < 255; x++) {
                SetPixel(hdc, x+7, y+7, RGB(x,y,255-(x+y)/2));
            }
        }
        for (int y = 0; y < 16; y++) {
            for (int x = 255+16; x < 255+32; x++) {
                SetPixel(hdc, x+7, y+7, RGB(data->r, data->g, data->b));
            }
        }
        EndPaint(hDlg, &ps);
        break;
    }
    case WM_LBUTTONDOWN:
    {
        data->ifMouseDown = TRUE;  // 标记鼠标按下
        int x = LOWORD(lParam);
        int y = HIWORD(lParam);
        data->r = x;
        data->g = y;
        data->b = 255 - (x + y) / 2;
        RECT rect = { 255 + 16 + 7, 7,255 + 32 + 7,16 + 7 };
        InvalidateRect(hDlg, &rect, TRUE);
        return (INT_PTR)TRUE;
    }
    case WM_DESTROY:
    {
        delete data;
        EndDialog(hDlg, NULL);
        return (INT_PTR)TRUE;
    }
    }

    return (INT_PTR)FALSE;
}
