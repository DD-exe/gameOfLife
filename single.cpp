/*
    用于实现“单机模拟”对话框消息处理
*/
#include "framework.h"
#include "gameOfLife.h"

INT_PTR CALLBACK single(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    siData* data = (siData*)GetWindowLongPtr(hDlg, GWLP_USERDATA);
    switch (message)
    {
    case WM_INITDIALOG:
    {
        siData* data = new siData();
        data->ifRun = FALSE;
        data->cellSize = 10;
        data->rule.x = 2; data->rule.y = 3;
        data->rule.z = 3; data->rule.t = 3;
        data->speed = 10;
        data->ifCreate = FALSE;
        data->ifMouseDown = FALSE;
        data->lastX = data->lastY = -1;                         //TODO: done
        data->listHalfSize = 150;
        data->listUnitHeight = 40;
        data->titleSize = 80;
        data->playerColor = RGBpurple;
        SetTimer(hDlg, ID_TIMER3, 100 * data->speed, NULL);      // 初始化计时器，WM_TIMER需要
        SetWindowLongPtr(hDlg, GWLP_USERDATA, (LONG_PTR)data);
        SetSingleWindows(hDlg, data);
        
        return (INT_PTR)TRUE;
    }
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        // 菜单选择
        switch (wmId)
        {
        case IDM_ABOUT:
        {
            DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_RULES), hDlg, Rules);
        }
        break;
        case ID_START:
        {
            data->ifRun = !data->ifRun;
            if (data->ifRun) {
                SetWindowText(data->doingInfo, L"||");
            }
            else {
                SetWindowText(data->doingInfo, L">");
            }
        }
        break;
        case ID_STOP:
        {
            data->grid.clear(); data->ifCreate = FALSE;
            RECT rect = { 0,0,data->tableX * data->cellSize,data->tableY * data->cellSize };
            InvalidateRect(hDlg, &rect, TRUE);
        }
        break;
        case ID_EDIT1OK:
        {
            BOOL success; data->ifCreate = FALSE;
            INT x = GetDlgItemInt(hDlg, ID_EDIT1, &success, TRUE);
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
                // COLORREF it=SetTextColor(GetDC(cellsizeInfo), RGB(255, 255, 0));
                SetWindowText(data->cellsizeInfo, std::to_wstring(data->cellSize).c_str());
                InvalidateRect(hDlg, &rect, TRUE);
            }
        }
        break;
        case ID_EDIT2OK:
        {
            BOOL success;
            data->speed = GetDlgItemInt(hDlg, ID_EDIT2, &success, TRUE);
            if (success) {
                KillTimer(hDlg, ID_TIMER3);
                SetTimer(hDlg, ID_TIMER3, 100 * data->speed, NULL);
                SetWindowText(data->timeInfo, std::to_wstring(data->speed).c_str());
            }
        }
        break;
        case ID_EDIT3OK:
        {
            BOOL success1, success2;
            INT x = GetDlgItemInt(hDlg, ID_EDIT3X, &success1, TRUE);
            INT y = GetDlgItemInt(hDlg, ID_EDIT3Y, &success2, TRUE);
            if (success1 && success2) {
                if (x < y) {
                    data->rule.x = x; data->rule.y = y;
                    SetWindowText(data->modXuInfo, (std::to_wstring(x) + L"-" + std::to_wstring(y)).c_str());
                }
                else if (x == y) {
                    data->rule.x = x; data->rule.y = y;
                    SetWindowText(data->modXuInfo, std::to_wstring(x).c_str());
                }
            }
        }
        break;
        case ID_EDIT4OK:
        {
            BOOL success1, success2;
            INT x = GetDlgItemInt(hDlg, ID_EDIT4X, &success1, TRUE);
            INT y = GetDlgItemInt(hDlg, ID_EDIT4Y, &success2, TRUE);
            if (success1 && success2) {
                if (x < y) {
                    data->rule.z = x; data->rule.t = y;
                    SetWindowText(data->modReInfo, (std::to_wstring(x) + L"-" + std::to_wstring(y)).c_str());
                }
                else if (x == y) {
                    data->rule.z = x; data->rule.t = y;
                    SetWindowText(data->modReInfo, std::to_wstring(x).c_str());
                }
            }
        }
        break;
        case IDM_SAVE:
        {
            saveGrid(hDlg, data->grid);
        }
        break;
        case ID_SAVE:
        {
            saveBmp(hDlg, 0, 0, data->cellSize * data->tableX, data->cellSize * data->tableY);
            return (INT_PTR)TRUE;
        }
        case IDM_LOAD:
        {
            RECT rect = { 0, 0,data->tableX * data->cellSize, data->tableY * data->cellSize };
            data->grid = std::move(loadGrid(hDlg));
            data->ifCreate = FALSE;
            InvalidateRect(hDlg, &rect, TRUE);
        }
        break;
        case IDCANCEL:
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        return (INT_PTR)TRUE;
    }
    case WM_TIMER:
    {
        if (data->ifRun && wParam == ID_TIMER3) {
            std::unordered_map<INT, std::unordered_map<INT, BOOL>> ans;
            // INT4 rectx;
            myLife(data->grid, ans, data->rule, state);
            RECT rect = { 0, 0,data->tableX * data->cellSize, data->tableY * data->cellSize };
            data->grid = std::move(ans);
            InvalidateRect(hDlg, &rect, TRUE);
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
        data->colorBlockX = clientWidth - 50 - data->listHalfSize - data->titleSize;
        moveSingleWindows(hDlg, data,clientWidth);
        InvalidateRect(hDlg, NULL, TRUE);
        return (INT_PTR)TRUE;
    }
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hDlg, &ps);
        HBRUSH hBrushLive = CreateSolidBrush(data->playerColor);
        HBRUSH hBrushDead = CreateSolidBrush(RGBwhite);
        RECT rectFull = { 0,0,data->tableX * data->cellSize, data->tableY * data->cellSize };
        FillRect(hdc, &rectFull, hBrushDead);
        for (int y = 0; y < data->tableY; y++) {
            for (int x = 0; x < data->tableX; x++) {
                RECT rect = { x * data->cellSize, y * data->cellSize,
                    (x + 1) * data->cellSize, (y + 1) * data->cellSize };
                if (findLife(data->grid, x, y)) FillRect(hdc, &rect, hBrushLive);
            }
        }
        RECT rect = { data->colorBlockX,data->colorBlockY,
            data->colorBlockX + data->colorBlockSize,
            data->colorBlockY + data->colorBlockSize };
        FillRect(hdc, &rect, hBrushLive);
        if (!data->ifCreate) {
            Gdiplus::Graphics graphics(hdc);
            myPaintFrame(graphics, 0, 0, data->tableX * data->cellSize, data->tableY * data->cellSize, data->cellSize);
            data->ifCreate = TRUE;
        }
        DeleteObject(hBrushLive);
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
            exchangeLife(data->grid, x, y);
            RECT rect = { x * data->cellSize, y * data->cellSize,
                        (x + 1) * data->cellSize, (y + 1) * data->cellSize };
            InvalidateRect(hDlg, &rect, TRUE);  // 仅重绘该区域
            data->lastX = x; data->lastY = y;  // 记录上次处理的格子，避免 `WM_MOUSEMOVE` 立即重复处理
        }
        else if (LOWORD(lParam) >= data->colorBlockX && LOWORD(lParam) < data->colorBlockX + data->colorBlockSize && HIWORD(lParam) < data->colorBlockY + data->colorBlockSize) {
            data->playerColor = DialogBoxParam(GetModuleHandle(NULL),
                MAKEINTRESOURCE(IDD_COLOR), hDlg, color, (LPARAM)(data->playerColor));
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
                exchangeLife(data->grid, x, y);
                RECT rect = { x * data->cellSize, y * data->cellSize, 
                    (x + 1) * data->cellSize, (y + 1) * data->cellSize };
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
    case WM_DESTROY:
    {
        delete data;
        KillTimer(hDlg, ID_TIMER3);
        EndDialog(hDlg, LOWORD(wParam));
        return (INT_PTR)TRUE;
    }
    }
    return (INT_PTR)FALSE;
}

void SetSingleWindows(HWND hDlg, siData* data) {
    INT clientWidth, clientHeight;
    getClientXY(hDlg, &clientWidth, &clientHeight);
    data->tableX = (clientWidth - 2 * data->listHalfSize - 10) / data->cellSize;
    data->tableY = clientHeight / data->cellSize;           // 计算右边控制栏位置
    // 插入一系列控件
    data->startBotton = CreateWindow(
        L"BUTTON", L"启动/暂停(O)",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        clientWidth - 50 - data->listHalfSize, data->listUnitHeight * 0, 100, 30,
        hDlg, (HMENU)ID_START, NULL, NULL
    );
    data->stopBotton = CreateWindow(
        L"BUTTON", L"重置(U)",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        clientWidth - 50 - data->listHalfSize, data->listUnitHeight * 1, 100, 30,
        hDlg, (HMENU)ID_STOP, NULL, NULL
    );
    data->doingInfo = CreateWindow(TEXT("STATIC"), TEXT(">"),
        WS_CHILD | WS_VISIBLE | SS_CENTERIMAGE | SS_CENTER,
        clientWidth - data->listHalfSize + 50, data->listUnitHeight * 0, 30, 30,
        hDlg, (HMENU)ID_DOING, (HINSTANCE)GetWindowLongPtr(hDlg, GWLP_HINSTANCE), NULL);

    data->cellsizeTitle = CreateWindow(TEXT("STATIC"), TEXT("设置网格："),
        WS_CHILD | WS_VISIBLE | SS_CENTER,
        clientWidth - 50 - data->listHalfSize - data->titleSize, data->listUnitHeight * 2, data->titleSize, 30,
        hDlg, (HMENU)ID_TITLE1, (HINSTANCE)GetWindowLongPtr(hDlg, GWLP_HINSTANCE), NULL);
    data->cellsizeEdit = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"10",
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
        clientWidth - 50 - data->listHalfSize, data->listUnitHeight * 2, 50, 30,
        hDlg, (HMENU)ID_EDIT1, (HINSTANCE)GetWindowLongPtr(hDlg, GWLP_HINSTANCE), NULL
    );
    data->cellsizeBotton = CreateWindow(
        L"BUTTON", L"确认",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        clientWidth - data->listHalfSize, data->listUnitHeight * 2, 50, 30,
        hDlg, (HMENU)ID_EDIT1OK, NULL, NULL
    );
    data->cellsizeInfo = CreateWindow(TEXT("STATIC"), TEXT(""),
        WS_CHILD | WS_VISIBLE | SS_CENTERIMAGE | SS_CENTER,
        clientWidth - data->listHalfSize + 50, data->listUnitHeight * 2, 30, 30,
        hDlg, (HMENU)ID_OUTPUT1, (HINSTANCE)GetWindowLongPtr(hDlg, GWLP_HINSTANCE), NULL);
    SetWindowText(data->cellsizeInfo, std::to_wstring(data->cellSize).c_str());

    data->timeTitle = CreateWindow(TEXT("STATIC"), TEXT("迭代用时："),
        WS_CHILD | WS_VISIBLE | SS_CENTER,
        clientWidth - 50 - data->listHalfSize - data->titleSize, data->listUnitHeight * 3, data->titleSize, 30,
        hDlg, (HMENU)ID_TITLE2, (HINSTANCE)GetWindowLongPtr(hDlg, GWLP_HINSTANCE), NULL);
    data->timeEdit = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"10",
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
        clientWidth - 50 - data->listHalfSize, data->listUnitHeight * 3, 50, 30,
        hDlg, (HMENU)ID_EDIT2, (HINSTANCE)GetWindowLongPtr(hDlg, GWLP_HINSTANCE), NULL
    );
    data->timeBotton = CreateWindow(
        L"BUTTON", L"确认",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        clientWidth - data->listHalfSize, data->listUnitHeight * 3, 50, 30,
        hDlg, (HMENU)ID_EDIT2OK, NULL, NULL
    );
    data->timeInfo = CreateWindow(TEXT("STATIC"), TEXT(""),
        WS_CHILD | WS_VISIBLE | SS_CENTERIMAGE | SS_CENTER,
        clientWidth - data->listHalfSize + 50, data->listUnitHeight * 3, 30, 30,
        hDlg, (HMENU)ID_OUTPUT2, (HINSTANCE)GetWindowLongPtr(hDlg, GWLP_HINSTANCE), NULL);
    SetWindowText(data->timeInfo, std::to_wstring(data->speed).c_str());

    data->modXuTitle = CreateWindow(TEXT("STATIC"), TEXT("存活需求："),
        WS_CHILD | WS_VISIBLE | SS_CENTER,
        clientWidth - 50 - data->listHalfSize - data->titleSize, data->listUnitHeight * 4, data->titleSize, 30,
        hDlg, (HMENU)ID_TITLE3, (HINSTANCE)GetWindowLongPtr(hDlg, GWLP_HINSTANCE), NULL);
    data->modXuEditX = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"2",
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
        clientWidth - 50 - data->listHalfSize, data->listUnitHeight * 4, 25, 30,
        hDlg, (HMENU)ID_EDIT3X, (HINSTANCE)GetWindowLongPtr(hDlg, GWLP_HINSTANCE), NULL
    );
    data->modXuEditY = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"3",
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
        clientWidth - 25 - data->listHalfSize, data->listUnitHeight * 4, 25, 30,
        hDlg, (HMENU)ID_EDIT3Y, (HINSTANCE)GetWindowLongPtr(hDlg, GWLP_HINSTANCE), NULL
    );
    data->modXuBotton = CreateWindow(
        L"BUTTON", L"确认",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        clientWidth - data->listHalfSize, data->listUnitHeight * 4, 50, 30,
        hDlg, (HMENU)ID_EDIT3OK, NULL, NULL
    );
    data->modXuInfo = CreateWindow(TEXT("STATIC"), TEXT("2-3"),
        WS_CHILD | WS_VISIBLE | SS_CENTERIMAGE | SS_CENTER,
        clientWidth - data->listHalfSize + 50, data->listUnitHeight * 4, 50, 30,
        hDlg, (HMENU)ID_OUTPUT3, (HINSTANCE)GetWindowLongPtr(hDlg, GWLP_HINSTANCE), NULL);

    data->modReTitle = CreateWindow(TEXT("STATIC"), TEXT("繁殖需求："),
        WS_CHILD | WS_VISIBLE | SS_CENTER,
        clientWidth - 50 - data->listHalfSize - data->titleSize, data->listUnitHeight * 5, data->titleSize, 30,
        hDlg, (HMENU)ID_TITLE4, (HINSTANCE)GetWindowLongPtr(hDlg, GWLP_HINSTANCE), NULL);
    data->modReEditX = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"3",
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
        clientWidth - 50 - data->listHalfSize, data->listUnitHeight * 5, 25, 30,
        hDlg, (HMENU)ID_EDIT4X, (HINSTANCE)GetWindowLongPtr(hDlg, GWLP_HINSTANCE), NULL
    );
    data->modReEditY = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"3",
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
        clientWidth - 25 - data->listHalfSize, data->listUnitHeight * 5, 25, 30,
        hDlg, (HMENU)ID_EDIT4Y, (HINSTANCE)GetWindowLongPtr(hDlg, GWLP_HINSTANCE), NULL
    );
    data->modReBotton = CreateWindow(
        L"BUTTON", L"确认",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        clientWidth - data->listHalfSize, data->listUnitHeight * 5, 50, 30,
        hDlg, (HMENU)ID_EDIT4OK, NULL, NULL
    );
    data->modReInfo = CreateWindow(TEXT("STATIC"), TEXT("3"),
        WS_CHILD | WS_VISIBLE | SS_CENTERIMAGE | SS_CENTER,
        clientWidth - data->listHalfSize + 50, data->listUnitHeight * 5, 50, 30,
        hDlg, (HMENU)ID_OUTPUT4, (HINSTANCE)GetWindowLongPtr(hDlg, GWLP_HINSTANCE), NULL);

    HBITMAP hBmp = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_LINbmpPro));
    data->hBmpStatic = CreateWindow(
        L"STATIC", NULL,
        WS_CHILD | WS_VISIBLE | SS_BITMAP,
        clientWidth - 2 * data->listHalfSize, data->listUnitHeight * 6, 2 * data->listHalfSize, 3 * data->listHalfSize,
        hDlg, NULL, NULL, NULL
    );
    SendMessage(data->hBmpStatic, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hBmp);// 关联 BMP 图片到静态控件       
    data->colorBlockX = clientWidth - 50 - data->listHalfSize - data->titleSize;
    data->colorBlockY = 0;
    data->colorBlockSize = 64;
}

void moveSingleWindows(HWND hDlg, siData* data,INT clientWidth) {
    MoveWindow(data->startBotton, clientWidth - 50 - data->listHalfSize,
        data->listUnitHeight * 0, 100, 30, TRUE);
    MoveWindow(data->stopBotton, clientWidth - 50 - data->listHalfSize,
        data->listUnitHeight * 1, 100, 30, TRUE);
    MoveWindow(data->doingInfo, clientWidth - data->listHalfSize + 50,
        data->listUnitHeight * 0, 30, 30, TRUE);

    MoveWindow(data->cellsizeTitle, clientWidth - 50 - data->listHalfSize - data->titleSize,
        data->listUnitHeight * 2, data->titleSize, 30, TRUE);
    MoveWindow(data->cellsizeEdit, clientWidth - 50 - data->listHalfSize,
        data->listUnitHeight * 2, 50, 30, TRUE);
    MoveWindow(data->cellsizeBotton, clientWidth - data->listHalfSize,
        data->listUnitHeight * 2, 50, 30, TRUE);
    MoveWindow(data->cellsizeInfo, clientWidth - data->listHalfSize + 50,
        data->listUnitHeight * 2, 30, 30, TRUE);

    MoveWindow(data->timeTitle, clientWidth - 50 - data->listHalfSize - data->titleSize,
        data->listUnitHeight * 3, data->titleSize, 30, TRUE);
    MoveWindow(data->timeEdit, clientWidth - 50 - data->listHalfSize,
        data->listUnitHeight * 3, 50, 30, TRUE);
    MoveWindow(data->timeBotton, clientWidth - data->listHalfSize,
        data->listUnitHeight * 3, 50, 30, TRUE);
    MoveWindow(data->timeInfo, clientWidth - data->listHalfSize + 50,
        data->listUnitHeight * 3, 30, 30, TRUE);

    MoveWindow(data->modXuTitle, clientWidth - 50 - data->listHalfSize - data->titleSize,
        data->listUnitHeight * 4, data->titleSize, 30, TRUE);
    MoveWindow(data->modXuEditX, clientWidth - 50 - data->listHalfSize,
        data->listUnitHeight * 4, 25, 30, TRUE);
    MoveWindow(data->modXuEditY, clientWidth - 25 - data->listHalfSize,
        data->listUnitHeight * 4, 25, 30, TRUE);
    MoveWindow(data->modXuBotton, clientWidth - data->listHalfSize,
        data->listUnitHeight * 4, 50, 30, TRUE);
    MoveWindow(data->modXuInfo, clientWidth - data->listHalfSize + 50,
        data->listUnitHeight * 4, 50, 30, TRUE);

    MoveWindow(data->modReTitle, clientWidth - 50 - data->listHalfSize - data->titleSize,
        data->listUnitHeight * 5, data->titleSize, 30, TRUE);
    MoveWindow(data->modReEditX, clientWidth - 50 - data->listHalfSize,
        data->listUnitHeight * 5, 25, 30, TRUE);
    MoveWindow(data->modReEditY, clientWidth - 25 - data->listHalfSize,
        data->listUnitHeight * 5, 25, 30, TRUE);
    MoveWindow(data->modReBotton, clientWidth - data->listHalfSize,
        data->listUnitHeight * 5, 50, 30, TRUE);
    MoveWindow(data->modReInfo, clientWidth - data->listHalfSize + 50,
        data->listUnitHeight * 5, 50, 30, TRUE);

    MoveWindow(data->hBmpStatic, clientWidth - 2 * data->listHalfSize,
        data->listUnitHeight * 6, 2 * data->listHalfSize, 3 * data->listHalfSize, TRUE);
}