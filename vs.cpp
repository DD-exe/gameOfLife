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
        data->ifRun = FALSE;
        data->ifMouseDown = FALSE;
        data->lastX = data->lastY = -1;
        data->speed = 10;
        data->cellSize = 10;
        data->listHalfSize = 80;
        data->ruleP1.x = 2; data->ruleP1.y = 3; data->ruleP1.z = 3; data->ruleP1.t = 3;
        data->ruleP2.x = 2; data->ruleP2.y = 3; data->ruleP2.z = 3; data->ruleP2.t = 3;
        data->attP1 = 3; data->attP2 = 3;
        data->defP1 = 3; data->defP2 = 3;
        data->moveX = data->moveY = 0;
        INT clientWidth, clientHeight;
        getClientXY(hDlg, &clientWidth, &clientHeight);
        data->tableX = (clientWidth - 2 * data->listHalfSize) / data->cellSize;
        data->tableY = clientHeight / data->cellSize;                           // 计算右边控制栏位置
        HWND hCombo = GetDlgItem(hDlg, IDC_CHARA);                             
        SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)L"绿色");
        SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)L"紫色");
        SendMessage(hCombo, CB_SETCURSEL, 0, 0);                                // 选择第一个选项
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
        case IDvsSTART:
        {
            data->ifRun = !data->ifRun;
            break;
        }
        case IDvsSTOP:
        {
            data->gridP1.clear();
            data->gridP2.clear();
            data->ifCreate = FALSE;
            RECT rect = { 0,0,data->tableX * data->cellSize,data->tableY * data->cellSize };
            InvalidateRect(hDlg, &rect, TRUE);
            break;
        }
        case IDvsUP:
        {
            if (data->moveX > 0)--(data->moveX);
            RECT rect = { 0,0,data->tableX * data->cellSize,data->tableY * data->cellSize };
            InvalidateRect(hDlg, &rect, TRUE);
            return (INT_PTR)TRUE;
        }
        case IDvsDOWN:
        {
            ++(data->moveX);
            RECT rect = { 0,0,data->tableX * data->cellSize,data->tableY * data->cellSize };
            InvalidateRect(hDlg, &rect, TRUE);
            return (INT_PTR)TRUE;
        }
        case IDvsLEFT:
        {
            if (data->moveY > 0)--(data->moveY);
            RECT rect = { 0,0,data->tableX * data->cellSize,data->tableY * data->cellSize };
            InvalidateRect(hDlg, &rect, TRUE);
            return (INT_PTR)TRUE;
        }
        case IDvsRIGHT:
        {
            ++(data->moveY);
            RECT rect = { 0,0,data->tableX * data->cellSize,data->tableY * data->cellSize };
            InvalidateRect(hDlg, &rect, TRUE);
            return (INT_PTR)TRUE;
        }
        case IDCANCEL:
            EndDialog(hDlg, LOWORD(wParam));
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
        RECT rectFull = { 0,0,data->tableX * data->cellSize, data->tableY * data->cellSize };
        FillRect(hdc, &rectFull, hBrushDead);
        for (int y = 0; y < data->tableY; y++) {
            for (int x = 0; x < data->tableX; x++) {
                RECT rect = { x * data->cellSize, y * data->cellSize,
                    (x + 1) * data->cellSize, (y + 1) * data->cellSize };
                if (findLife(data->gridP1,x,y)) {
                    if (findLife(data->gridP2, x, y)) {
                        INT p1vs = data->gridP1[y][x] ? data->attP1 : data->defP1;
                        INT p2vs = data->gridP2[y][x] ? data->attP2 : data->defP2;
                        INT ans = getRandomNum(1, p1vs + p2vs);
                        if (ans <= p1vs) { 
                            exchangeLife(data->gridP2, x, y);
                            FillRect(hdc, &rect, hBrushP1);
                        }
                        else {
                            exchangeLife(data->gridP1, x, y);
                            FillRect(hdc, &rect, hBrushP2);
                        }
                    }
                    else {
                        FillRect(hdc, &rect, hBrushP1);
                    }
                }
                else if (findLife(data->gridP2, x, y)) {
                    FillRect(hdc, &rect, hBrushP2);
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
    case WM_LBUTTONDOWN:
    {
        data->ifMouseDown = TRUE;  // 标记鼠标按下
        int x = LOWORD(lParam) / data->cellSize;
        int y = HIWORD(lParam) / data->cellSize;
        if (x < data->tableX && y < data->tableY) {
            HWND hChara = GetDlgItem(hDlg, IDC_CHARA);
            int index = SendMessage(hChara, CB_GETCURSEL, 0, 0); // 获取选中索引
            if (index == 0) exchangeLife(data->gridP1, x, y);
            else exchangeLife(data->gridP2, x, y);
            RECT rect = { x * data->cellSize, y * data->cellSize,
                        (x + 1) * data->cellSize, (y + 1) * data->cellSize };
            InvalidateRect(hDlg, &rect, TRUE);  // 仅重绘该区域
            data->lastX = x; data->lastY = y;  // 记录上次处理的格子，避免 `WM_MOUSEMOVE` 立即重复处理
        }
        return (INT_PTR)TRUE;
    }
    case WM_MOUSEMOVE:
    {
        if (data->ifMouseDown) {  // 仅在鼠标按住时处理
            int x = LOWORD(lParam) / data->cellSize;
            int y = HIWORD(lParam) / data->cellSize;

            if (x < data->tableX && y < data->tableY && (x != data->lastX || y != data->lastY)) {
                // 只有当鼠标进入新格子时才处理，防止重复
                HWND hChara = GetDlgItem(hDlg, IDC_CHARA);
                int index = SendMessage(hChara, CB_GETCURSEL, 0, 0); // 获取选中索引
                if (index == 0) exchangeLife(data->gridP1, x, y);
                else exchangeLife(data->gridP2, x, y);
                RECT rect = { x * data->cellSize, y * data->cellSize, (x + 1) * data->cellSize, (y + 1) * data->cellSize };
                InvalidateRect(hDlg, &rect, TRUE);

                data->lastX = x;
                data->lastY = y;  // 记录上次处理的格子
            }
        }
        return (INT_PTR)TRUE;
    }
    case WM_LBUTTONUP:
    {
        data->ifMouseDown = FALSE;  // 释放鼠标按下状态
        data->lastX = data->lastY = -1;   // 清除上次处理的格子记录
        return (INT_PTR)TRUE;
    }        
    case WM_TIMER:
    {
        if (data->ifRun&&wParam == ID_TIMER2)
        {
            std::unordered_map<INT, std::unordered_map<INT, BOOL>> ans1;
            std::unordered_map<INT, std::unordered_map<INT, BOOL>> ans2;
            myLife(data->gridP1, ans1, data->ruleP1, state);
            myLife(data->gridP2, ans2, data->ruleP2, state);
            RECT rect = { 0, 0,data->tableX * data->cellSize, data->tableY * data->cellSize };
            data->gridP1 = std::move(ans1);
            data->gridP2 = std::move(ans2);
            InvalidateRect(hDlg, &rect, TRUE);
            break;
        }
        return (INT_PTR)TRUE;
    }        
    case WM_DESTROY:
    {  
        delete data;
        KillTimer(hDlg, ID_TIMER2);
        EndDialog(hDlg, LOWORD(wParam));
        return (INT_PTR)TRUE;
    }        
    }
    return (INT_PTR)FALSE;
}