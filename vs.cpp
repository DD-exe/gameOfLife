#include "framework.h"
#include "gameOfLife.h"

INT_PTR CALLBACK VSdot(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    vsData* data = (vsData*)GetWindowLongPtr(hDlg, GWLP_USERDATA);
    switch (message)
    {
    case WM_INITDIALOG:
    {
        vsData* data = new vsData();
        SetTimer(hDlg, ID_TIMER2, 1000, NULL);
        SetWindowLongPtr(hDlg, GWLP_USERDATA, (LONG_PTR)data);
        return (INT_PTR)TRUE;
    }
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        switch (wmId)
        {
        case IDOKvsSIZE:
        {
            BOOL success; data->ifCreate = FALSE;
            INT x = GetDlgItemInt(hDlg, ID_EDIT1, &success, TRUE);
        }
        case IDCANCEL:
            EndDialog(hDlg, LOWORD(wParam));
            delete data;
            return (INT_PTR)TRUE;
        default:
            break;
        }
        break;
    }  
    case WM_TIMER:
        if (wParam == ID_TIMER2)
        {
            
        }
        break;
    }
    return (INT_PTR)FALSE;
}