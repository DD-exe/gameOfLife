#include "framework.h"
#include "gameOfLife.h"
#define RGBgrey     RGB(200,200,200)
#define RGBblack    RGB(0,0,0)
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
        data->rule[0].x = 2; data->rule[0].y = 3; data->rule[0].z = 3; data->rule[0].t = 3;
        data->rule[1].x = 2; data->rule[1].y = 3; data->rule[1].z = 3; data->rule[1].t = 3;
        // data->rule[2].x = 2; data->rule[2].y = 3; data->rule[2].z = 3; data->rule[2].t = 3;
        // data->rule[3].x = 2; data->rule[3].y = 3; data->rule[3].z = 3; data->rule[3].t = 3;
        data->att[0] = 3; data->def[0] = 3;
        data->att[1] = 3; data->def[1] = 3;
        // data->att[2] = 3; data->def[2] = 3;
        // data->att[3] = 3; data->def[3] = 3;
        data->moveX = data->moveY = 0;
        INT clientWidth, clientHeight;
        getClientXY(hDlg, &clientWidth, &clientHeight);
        data->tableX = (clientWidth - 2 * data->listHalfSize) / data->cellSize;
        data->tableY = clientHeight / data->cellSize;                           // 计算右边控制栏位置
        data->playerColor[0] = RGBgreen;
        data->playerColor[1] = RGBpurple;
        data->colorBlockX = 800;
        data->colorBlockY = 105;
        data->colorBlockSize = 16;
        // data->player[2] = RGBgrey;
        // data->player[3] = RGBblack;
        HWND hCombo = GetDlgItem(hDlg, IDC_CHARA);  
        SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)L"阵营1");
        SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)L"阵营2");
        // SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)L"阵营三");
        // SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)L"阵营四");
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
        case IDPlayerOK:
        {
            BOOL success1, success2,success3,success4;
            INT p1att = GetDlgItemInt(hDlg, IDC_P1ATT, &success1, TRUE);
            INT p1def = GetDlgItemInt(hDlg, IDC_P1DEF, &success2, TRUE);
            INT p1muv = GetDlgItemInt(hDlg, IDC_P1MUV, &success3, TRUE);
            INT p1suv = GetDlgItemInt(hDlg, IDC_P1SUV, &success4, TRUE);
            HWND hChara = GetDlgItem(hDlg, IDC_CHARA);
            int index = SendMessage(hChara, CB_GETCURSEL, 0, 0);
            if (success1&&success2&&success3&&success4) {
                data->att[index] = p1att;
                data->def[index] = p1def;
                switch (p1muv)
                {
                case 1:
                    data->rule[index].z = 3; data->rule[index].t = 3;
                    break;
                case 2:
                    data->rule[index].z = 3; data->rule[index].t = 4;
                    break;
                case 3:
                    data->rule[index].z = 2; data->rule[index].t = 4;
                    break;
                default:
                    break;
                }
                switch (p1suv)
                {
                case 1:
                    data->rule[index].x = 2; data->rule[index].y = 3;
                    break;
                case 2:
                    data->rule[index].x = 2; data->rule[index].y = 4;
                    break;
                case 3:
                    data->rule[index].x = 1; data->rule[index].y = 4;
                    break;
                default:
                    break;
                }
            }
            break;
        }
        case ID_START:
        {
            data->ifRun = !data->ifRun;
            break;
        }
        case ID_STOP:
        {
            data->grid[0].clear();
            data->grid[1].clear();
            data->ifCreate = FALSE;
            RECT rect = { 0,0,data->tableX * data->cellSize,data->tableY * data->cellSize };
            InvalidateRect(hDlg, &rect, TRUE);
            break;
        }
        case IDM_SAVE:
        {
            saveVSGrid(hDlg, data->grid);
            return (INT_PTR)TRUE;
        }
        case ID_SAVE:
        {
            saveBmp(hDlg, 0,0,data->cellSize*data->tableX, data->cellSize* data->tableY);
            return (INT_PTR)TRUE;
        }
        case IDM_LOAD:
        {
            loadVSGrid(hDlg, data->grid);
            RECT rect = { 0, 0,data->tableX * data->cellSize, data->tableY * data->cellSize };
            data->ifCreate = FALSE;
            InvalidateRect(hDlg, &rect, TRUE);
            MessageBox(hDlg, L"成功加载文件: " , L"加载成功", MB_ICONINFORMATION);
            return (INT_PTR)TRUE;
        }
        case IDvsUP:
        {
            if (data->moveY > 0)--(data->moveY);
            RECT rect = { 0,0,data->tableX * data->cellSize,data->tableY * data->cellSize };
            InvalidateRect(hDlg, &rect, TRUE);
            return (INT_PTR)TRUE;
        }
        case IDvsDOWN:
        {
            ++(data->moveY);
            RECT rect = { 0,0,data->tableX * data->cellSize,data->tableY * data->cellSize };
            InvalidateRect(hDlg, &rect, TRUE);
            return (INT_PTR)TRUE;
        }
        case IDvsLEFT:
        {
            if (data->moveX > 0)--(data->moveX);
            RECT rect = { 0,0,data->tableX * data->cellSize,data->tableY * data->cellSize };
            InvalidateRect(hDlg, &rect, TRUE);
            return (INT_PTR)TRUE;
        }
        case IDvsRIGHT:
        {
            ++(data->moveX);
            RECT rect = { 0,0,data->tableX * data->cellSize,data->tableY * data->cellSize };
            InvalidateRect(hDlg, &rect, TRUE);
            return (INT_PTR)TRUE;
        }
        case IDC_CHARA: 
        {
            RECT rect = { 800,105,816,121 };
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
    case WM_SIZE:
    {
        int clientWidth = LOWORD(lParam);
        int clientHeight = HIWORD(lParam);
        data->tableX = (clientWidth - 2 * data->listHalfSize - 10) / data->cellSize;
        data->tableY = clientHeight / data->cellSize;
        data->ifCreate = FALSE;
        InvalidateRect(hDlg, NULL, TRUE);
        return (INT_PTR)TRUE;
    }
    case WM_PAINT: 
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hDlg, &ps);
        HBRUSH hBrushP1 = CreateSolidBrush(data->playerColor[0]);
        HBRUSH hBrushP2 = CreateSolidBrush(data->playerColor[1]);
        HBRUSH hBrushDead = CreateSolidBrush(RGBwhite);
        RECT rectFull = { 0,0,data->tableX * data->cellSize, data->tableY * data->cellSize };
        FillRect(hdc, &rectFull, hBrushDead);
        for (int y = 0; y < data->tableY; y++) {
            for (int x = 0; x < data->tableX; x++) {
                RECT rect = { x * data->cellSize, y * data->cellSize,
                    (x + 1) * data->cellSize, (y + 1) * data->cellSize };
                INT mx = x+data->moveX;
                INT my = y+data->moveY;
                if (findLife(data->grid[0], mx, my)) {
                    if (findLife(data->grid[1], mx, my)) {
                        INT p1vs = data->grid[0][my][mx] ? data->att[0] : data->def[0];
                        INT p2vs = data->grid[1][my][mx] ? data->att[1] : data->def[1];
                        INT ans = getRandomNum(1, p1vs + p2vs);
                        if (ans <= p1vs) { 
                            exchangeLife(data->grid[1], mx, my);
                            FillRect(hdc, &rect, hBrushP1);
                        }
                        else {
                            exchangeLife(data->grid[0], mx, my);
                            FillRect(hdc, &rect, hBrushP2);
                        }
                    }
                    else {
                        FillRect(hdc, &rect, hBrushP1);
                    }
                }
                else if (findLife(data->grid[1], mx, my)) {
                    FillRect(hdc, &rect, hBrushP2);
                }
            }
        }
        HWND hChara = GetDlgItem(hDlg, IDC_CHARA);
        RECT rect = { data->colorBlockX,data->colorBlockY,
            data->colorBlockX + data->colorBlockSize,
            data->colorBlockY + data->colorBlockSize };
        int index = SendMessage(hChara, CB_GETCURSEL, 0, 0);
        FillRect(hdc, &rect, CreateSolidBrush(data->playerColor[index]));
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
            exchangeLife(data->grid[index], x+data->moveX, y+data->moveY);
            RECT rect = { x * data->cellSize, y * data->cellSize,
                        (x + 1) * data->cellSize, (y + 1) * data->cellSize };
            InvalidateRect(hDlg, &rect, TRUE);  // 仅重绘该区域
            data->lastX = x; data->lastY = y;  // 记录上次处理的格子，避免 `WM_MOUSEMOVE` 立即重复处理
        }
        else if (LOWORD(lParam) >= 800 && LOWORD(lParam) < 816 && HIWORD(lParam) >= 105 && HIWORD(lParam) < 121) {
            HWND hChara = GetDlgItem(hDlg, IDC_CHARA);
            int index = SendMessage(hChara, CB_GETCURSEL, 0, 0);
            data->playerColor[index] = DialogBoxParam(GetModuleHandle(NULL), 
                MAKEINTRESOURCE(IDD_COLOR), hDlg, color, (LPARAM)(data->playerColor[index]));
            RECT rect = { data->colorBlockX,data->colorBlockY,
            data->colorBlockX + data->colorBlockSize,
            data->colorBlockY + data->colorBlockSize };
            InvalidateRect(hDlg, &rect, TRUE);
            
            return (INT_PTR)TRUE;
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
                exchangeLife(data->grid[index], x + data->moveX, y + data->moveY);
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
            myLife(data->grid[0], ans1, data->rule[0], state);
            myLife(data->grid[1], ans2, data->rule[1], state);
            RECT rect = { 0, 0,data->tableX * data->cellSize, data->tableY * data->cellSize };
            data->grid[0] = std::move(ans1);
            data->grid[1] = std::move(ans2);
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