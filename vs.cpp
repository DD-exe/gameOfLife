#include "framework.h"
#include "gameOfLife.h"
#define RGBgrey     RGB(200,200,200)
#define RGBwhite    RGB(255,255,255)
#define RGBgreen    RGB(60,255,176)
#define RGBpurple   RGB(180,150,255)

INT_PTR CALLBACK VSdot(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    vsData* data = (vsData*)GetWindowLongPtr(hDlg, GWLP_USERDATA);
    switch (message)
    {
    case WM_INITDIALOG:
    {
        vsData* data = new vsData();
        data->ifCreate = FALSE;
        data->speed = 10;
        data->cellSize = 10;
        data->listHalfSize = 80;
        data->ruleP1.x = 2; data->ruleP1.y = 3; data->ruleP1.z = 3; data->ruleP1.t = 3;
        data->ruleP2.x = 2; data->ruleP2.y = 3; data->ruleP2.z = 3; data->ruleP2.t = 3;
        data->attP1 = 3; data->attP2 = 3;
        data->defP1 = 3; data->defP2 = 3;
        INT clientWidth, clientHeight;
        getClientXY(hDlg, &clientWidth, &clientHeight);
        data->tableX = (clientWidth - 2 * data->listHalfSize) / data->cellSize;
        data->tableY = clientHeight / data->cellSize; // ¼ÆËãÓÒ±ß¿ØÖÆÀ¸Î»ÖÃ
        SetTimer(hDlg, ID_TIMER2, 100*data->speed, NULL);
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
            INT x = GetDlgItemInt(hDlg, IDC_EDITvsSIZE, &success, TRUE);
            INT oX = data->tableX * data->cellSize;
            INT oY = data->tableY * data->cellSize;
            if (success) {
                data->cellSize = x;
                INT clientWidth, clientHeight;
                getClientXY(hDlg, &clientWidth, &clientHeight);
                data->tableX = (clientWidth - 2 * data->listHalfSize - 10) / data->cellSize;
                data->tableY = clientHeight / data->cellSize;
                oX = data->tableX * data->cellSize > oX ? data->tableX * data->cellSize : oX;
                oY = data->tableY * data->cellSize > oY ? data->tableY * data->cellSize : oY;
                RECT rect = { 0,0,oX,oY };
                InvalidateRect(hDlg, &rect, TRUE);
            }
            break;
        }
        case IDOKvsSPEED:
        {
            BOOL success;
            data->speed = GetDlgItemInt(hDlg, IDC_EDITvsSPEED, &success, TRUE);
            if (success) {
                KillTimer(hDlg, ID_TIMER2);
                SetTimer(hDlg, ID_TIMER2, 100 * data->speed, NULL);
            }
            break;
        }
        case IDP1OK:
        {
            BOOL success1, success2,success3,success4;
            INT p1att = GetDlgItemInt(hDlg, IDC_P1ATT, &success1, TRUE);
            INT p1def = GetDlgItemInt(hDlg, IDC_P1DEF, &success2, TRUE);
            INT p1muv = GetDlgItemInt(hDlg, IDC_P1MUV, &success3, TRUE);
            INT p1suv = GetDlgItemInt(hDlg, IDC_P1SUV, &success4, TRUE);
            if (success1&&success2&&success3&&success4) {
                data->attP1 = p1att;
                data->defP1 = p1def;
                switch (p1muv)
                {
                case 1:
                    data->ruleP1.z = 3; data->ruleP1.t = 3;
                    break;
                case 2:
                    data->ruleP1.z = 3; data->ruleP1.t = 4;
                    break;
                case 3:
                    data->ruleP1.z = 2; data->ruleP1.t = 4;
                    break;
                default:
                    break;
                }
                switch (p1suv)
                {
                case 1:
                    data->ruleP1.x = 2; data->ruleP1.y = 3;
                    break;
                case 2:
                    data->ruleP1.x = 2; data->ruleP1.y = 4;
                    break;
                case 3:
                    data->ruleP1.x = 1; data->ruleP1.y = 4;
                    break;
                default:
                    break;
                }
            }
            break;
        }
        case IDP2OK:
        {
            BOOL success1, success2, success3, success4;
            INT p2att = GetDlgItemInt(hDlg, IDC_P2ATT, &success1, TRUE);
            INT p2def = GetDlgItemInt(hDlg, IDC_P2DEF, &success2, TRUE);
            INT p2muv = GetDlgItemInt(hDlg, IDC_P2MUV, &success3, TRUE);
            INT p2suv = GetDlgItemInt(hDlg, IDC_P2SUV, &success4, TRUE);
            if (success1 && success2 && success3 && success4) {
                data->attP2 = p2att;
                data->defP2 = p2def;
                switch (p2muv)
                {
                case 1:
                    data->ruleP2.z = 3; data->ruleP2.t = 3;
                    break;
                case 2:
                    data->ruleP2.z = 3; data->ruleP2.t = 4;
                    break;
                case 3:
                    data->ruleP2.z = 2; data->ruleP2.t = 4;
                    break;
                default:
                    break;
                }
                switch (p2suv)
                {
                case 1:
                    data->ruleP2.x = 2; data->ruleP2.y = 3;
                    break;
                case 2:
                    data->ruleP2.x = 2; data->ruleP2.y = 4;
                    break;
                case 3:
                    data->ruleP2.x = 1; data->ruleP2.y = 4;
                    break;
                default:
                    break;
                }
            }
            break;
        }
        case IDCANCEL:
            EndDialog(hDlg, LOWORD(wParam));
            delete data;
            return (INT_PTR)TRUE;
        default:
            break;
        }
        return (INT_PTR)TRUE;
    }
    case WM_PAINT: 
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hDlg, &ps);
        HBRUSH hBrushP1 = CreateSolidBrush(RGBgreen);
        HBRUSH hBrushP2 = CreateSolidBrush(RGBpurple);
        HBRUSH hBrushDead = CreateSolidBrush(RGBwhite);
        for (int y = 0; y < data->tableY; y++) {
            for (int x = 0; x < data->tableX; x++) {
                RECT rect = { x * data->cellSize, y * data->cellSize,
                    (x + 1) * data->cellSize, (y + 1) * data->cellSize };
                if (findLife(data->gridP1,x,y)) {
                    if (findLife(data->gridP2, x, y)) {
                        INT p1vs = data->gridP1[x][y] ? data->attP1 : data->defP1;
                        INT p2vs = data->gridP2[x][y] ? data->attP2 : data->defP2;
                        INT ans = getRandomNum(1, p1vs + p2vs);
                        if (ans <= p1vs) FillRect(hdc, &rect, hBrushP1);
                        else FillRect(hdc, &rect, hBrushP2);
                    }
                    else {
                        FillRect(hdc, &rect, hBrushP1);
                    }
                }
                else if (findLife(data->gridP2, x, y)) {
                    FillRect(hdc, &rect, hBrushP2);
                }
                else {
                    FillRect(hdc, &rect, hBrushDead);
                }
            }
        }
        if (!data->ifCreate) {
            Gdiplus::Graphics graphics(hdc);
            myPaintFrame(graphics, 0, 0, data->tableX * data->cellSize, data->tableY * data->cellSize, data->cellSize);
            data->ifCreate = TRUE;
        }
        DeleteObject(hBrushP1);
        DeleteObject(hBrushP2);
        DeleteObject(hBrushDead);
        EndPaint(hDlg, &ps);
        return (INT_PTR)TRUE;
    }
    case WM_TIMER:
        if (wParam == ID_TIMER2)
        {
            break;
        }
        return (INT_PTR)TRUE;
    }
    return (INT_PTR)FALSE;
}