#include "framework.h"
#include "gameOfLife.h"

// 在线对战设置窗口
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

// 在线对战运行窗口
INT_PTR CALLBACK VSOnlineDot(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    vsoData* data = (vsoData*)GetWindowLongPtr(hDlg, GWLP_USERDATA);
    switch (message)
    {
    case WM_INITDIALOG:
    {
        vsoData* data = new vsoData();
        data->ifCreate = FALSE;
        // data->ifRun = FALSE;
        data->ifMouseDown = FALSE;
        data->ifServer = FALSE;
        data->ifClient = FALSE;
        SetWindowText(GetDlgItem(hDlg, ID_CNT), L"未启动");
        data->lastX = data->lastY = -1;
        data->cellSize = 10;
        data->listHalfSize = 80;
        data->score[0] = 20;
        data->score[1] = 20;
        data->rule[0].x = 2; data->rule[0].y = 3; data->rule[0].z = 3; data->rule[0].t = 3;
        data->rule[1].x = 2; data->rule[1].y = 3; data->rule[1].z = 3; data->rule[1].t = 3;
        data->att[0] = 3; data->def[0] = 3;
        data->att[1] = 3; data->def[1] = 3;
        data->muv[0] = 1; data->suv[0] = 1;
        data->muv[1] = 1; data->suv[1] = 1;
        data->moveX = data->moveY = 0;
        INT clientWidth, clientHeight;
        getClientXY(hDlg, &clientWidth, &clientHeight);
        data->tableX = (clientWidth - 2 * data->listHalfSize) / data->cellSize;
        data->tableY = clientHeight / data->cellSize;                           // 计算右边控制栏位置
        data->playerColor[0] = RGBgreen;
        data->playerColor[1] = RGBpurple;
        data->colorBlockX = 800;
        data->colorBlockY = 60;
        data->colorBlockSize = 16;
        HWND hCombo = GetDlgItem(hDlg, IDC_CHARA);
        SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)L"己阵营");
        SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)L"他阵营");
        SendMessage(hCombo, CB_SETCURSEL, 0, 0);                                // 选择第一个选项
        SetDlgItemText(hDlg, ID_OUTPUT1, std::to_wstring(data->att[0]).c_str());
        SetDlgItemText(hDlg, ID_OUTPUT2, std::to_wstring(data->def[0]).c_str());
        SetDlgItemText(hDlg, ID_OUTPUT3, std::to_wstring(data->muv[0]).c_str());
        SetDlgItemText(hDlg, ID_OUTPUT4, std::to_wstring(data->suv[0]).c_str());
        SetDlgItemText(hDlg, ID_OUTPUT, std::to_wstring(data->score[0]).c_str());
        SetDlgItemText(hDlg, IDC_ROOMIP, L"127.0.0.1");
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
        // 对于下面一系列add/sub，若为客户端，需要将操作同步给服务端（？
        case ID_ADDATT:
        {
            HWND hChara = GetDlgItem(hDlg, IDC_CHARA);
            int index = SendMessage(hChara, CB_GETCURSEL, 0, 0);
            if (index == 0) {
                if (data->score[index] >= 2) {
                    data->score[index] -= 2;
                    data->att[index] += 1;
                    SetWindowText(GetDlgItem(hDlg, ID_OUTPUT1), std::to_wstring(data->att[index]).c_str());              
                    SetWindowText(GetDlgItem(hDlg, ID_OUTPUT), std::to_wstring(data->score[index]).c_str());
                }
            }            
            break;
        }
        case ID_ADDDEF:
        {
            HWND hChara = GetDlgItem(hDlg, IDC_CHARA);
            int index = SendMessage(hChara, CB_GETCURSEL, 0, 0);
            if (index == 0) {
                if (data->score[index] >= 2) {
                    data->score[index] -= 2;
                    data->def[index] += 1;
                    SetWindowText(GetDlgItem(hDlg, ID_OUTPUT2), std::to_wstring(data->def[index]).c_str());
                    SetWindowText(GetDlgItem(hDlg, ID_OUTPUT), std::to_wstring(data->score[index]).c_str());
                }
            }
            break;
        }
        case ID_ADDMUV:
        {
            HWND hChara = GetDlgItem(hDlg, IDC_CHARA);
            int index = SendMessage(hChara, CB_GETCURSEL, 0, 0);
            if (index == 0) {
                if (data->score[index] >= 100&&data->muv[index]<3) {
                    data->score[index] -= 100;
                    data->muv[index] += 1;
                }
                switch (data->muv[index])
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
                }
                SetWindowText(GetDlgItem(hDlg, ID_OUTPUT3), std::to_wstring(data->muv[index]).c_str());
                SetWindowText(GetDlgItem(hDlg, ID_OUTPUT), std::to_wstring(data->score[index]).c_str());
            }
            break;
        }
        case ID_ADDSUV:
        {
            HWND hChara = GetDlgItem(hDlg, IDC_CHARA);
            int index = SendMessage(hChara, CB_GETCURSEL, 0, 0);
            if (index == 0) {
                if (data->score[index] >= 100 && data->suv[index] < 3) {
                    data->score[index] -= 100;
                    data->suv[index] += 1;
                }
                switch (data->suv[index])
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
                }
                SetWindowText(GetDlgItem(hDlg, ID_OUTPUT4), std::to_wstring(data->suv[index]).c_str());
                SetWindowText(GetDlgItem(hDlg, ID_OUTPUT), std::to_wstring(data->score[index]).c_str());
            }
            break;
        }
        case ID_SUBATT:
        {
            HWND hChara = GetDlgItem(hDlg, IDC_CHARA);
            int index = SendMessage(hChara, CB_GETCURSEL, 0, 0);
            if (index == 0) {
                if (data->att[index] > 0) {
                    data->score[index] += 2;
                    data->att[index] -= 1;
                    SetWindowText(GetDlgItem(hDlg, ID_OUTPUT1), std::to_wstring(data->att[index]).c_str());
                    SetWindowText(GetDlgItem(hDlg, ID_OUTPUT), std::to_wstring(data->score[index]).c_str());
                }
            }
            break;
        }
        case ID_SUBDEF:
        {
            HWND hChara = GetDlgItem(hDlg, IDC_CHARA);
            int index = SendMessage(hChara, CB_GETCURSEL, 0, 0);
            if (index == 0) {
                if (data->def[index] > 0) {
                    data->score[index] += 2;
                    data->def[index] -= 1;
                    SetWindowText(GetDlgItem(hDlg, ID_OUTPUT2), std::to_wstring(data->def[index]).c_str());
                    SetWindowText(GetDlgItem(hDlg, ID_OUTPUT), std::to_wstring(data->score[index]).c_str());
                }
            }
            break;
        }
        case ID_SUBMUV:
        {
            HWND hChara = GetDlgItem(hDlg, IDC_CHARA);
            int index = SendMessage(hChara, CB_GETCURSEL, 0, 0);
            if (index == 0) {
                if (data->muv[index] > 1) {
                    data->score[index] += 100;
                    data->muv[index] -= 1;
                }
                switch (data->muv[index])
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
                }
                SetWindowText(GetDlgItem(hDlg, ID_OUTPUT3), std::to_wstring(data->muv[index]).c_str());
                SetWindowText(GetDlgItem(hDlg, ID_OUTPUT), std::to_wstring(data->score[index]).c_str());
            }
            break;
        }
        case ID_SUBSUV:
        {
            HWND hChara = GetDlgItem(hDlg, IDC_CHARA);
            int index = SendMessage(hChara, CB_GETCURSEL, 0, 0);
            if (index == 0) {
                if (data->suv[index] > 1) {
                    data->score[index] += 100;
                    data->suv[index] -= 1;
                }
                switch (data->suv[index])
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
                }
                SetWindowText(GetDlgItem(hDlg, ID_OUTPUT4), std::to_wstring(data->suv[index]).c_str());
                SetWindowText(GetDlgItem(hDlg, ID_OUTPUT), std::to_wstring(data->score[index]).c_str());
            }
            break;
        }
        case ID_START:
        {
            if (data->ifClient || data->ifServer) {

            }
            else {
                data->score[0] += 10;
                data->score[1] += 10;
                HWND hChara = GetDlgItem(hDlg, IDC_CHARA);
                int index = SendMessage(hChara, CB_GETCURSEL, 0, 0);
                SetWindowText(GetDlgItem(hDlg, ID_OUTPUT), std::to_wstring(data->score[index]).c_str());
                
                std::unordered_map<INT, std::unordered_map<INT, BOOL>> ans1;
                std::unordered_map<INT, std::unordered_map<INT, BOOL>> ans2;
                myLife(data->grid[0], ans1, data->rule[0], state);
                myLife(data->grid[1], ans2, data->rule[1], state);
                RECT rect = { 0, 0,data->tableX * data->cellSize, data->tableY * data->cellSize };
                data->grid[0] = std::move(ans1);
                data->grid[1] = std::move(ans2);
                data->ifCreate = FALSE;
                InvalidateRect(hDlg, &rect, TRUE);
            }
            break;
        }
        case ID_STOP:
        {
            if (data->ifClient || data->ifServer) {

            }
            else {
                data->grid[0].clear();
                data->grid[1].clear();
                data->ifCreate = FALSE;
                RECT rect = { 0,0,data->tableX * data->cellSize,data->tableY * data->cellSize };
                InvalidateRect(hDlg, &rect, TRUE);
            }
            break;
        }
        case ID_STARTSERVER:
        {
            EnableWindow(GetDlgItem(hDlg,ID_STARTCLIENT), FALSE);
            EnableWindow(GetDlgItem(hDlg, IDC_IPADDRESS1), FALSE);
            SetDlgItemText(hDlg, ID_CNT, L"等待中……");
            data->ifServer = runServer(data->grid[0]); // trans缺陷，只传一个？
            if (data->ifServer) {
                SetDlgItemText(hDlg, ID_CNT, L"已连接");
                EnableWindow(GetDlgItem(hDlg, ID_STARTSERVER), FALSE);
            }
            else {
                SetDlgItemText(hDlg, ID_CNT, L"无连接");
                EnableWindow(GetDlgItem(hDlg, ID_STARTCLIENT), TRUE);
                EnableWindow(GetDlgItem(hDlg, IDC_IPADDRESS1), TRUE);
            }           
            return (INT_PTR)TRUE;
        }
        case ID_STARTCLIENT:
        {
            BOOL success=GetDlgItemText(hDlg, IDC_ROOMIP, data->targetIP, 100);
            if (success) {
                EnableWindow(GetDlgItem(hDlg, ID_STARTSERVER), FALSE);
                data->ifClient = runClient(wc2s(data->targetIP).c_str(), data->grid[0], data->theMove);
                if (data->ifClient) {
                    SetDlgItemText(hDlg, ID_CNT, L"");
                }
                else {
                    EnableWindow(GetDlgItem(hDlg, ID_STARTSERVER), TRUE);
                }
            }
            return (INT_PTR)TRUE;
        }
        case IDM_SAVE:
        {
            saveVSGrid(hDlg, data->grid);
            return (INT_PTR)TRUE;
        }
        case ID_SAVE:
        {
            saveBmp(hDlg, 0, 0, data->cellSize * data->tableX, data->cellSize * data->tableY);
            return (INT_PTR)TRUE;
        }
        case IDM_LOAD:
        {
            loadVSGrid(hDlg, data->grid);
            RECT rect = { 0, 0,data->tableX * data->cellSize, data->tableY * data->cellSize };
            data->ifCreate = FALSE;
            InvalidateRect(hDlg, &rect, TRUE);
            MessageBox(hDlg, L"成功加载文件: ", L"加载成功", MB_ICONINFORMATION);
            return (INT_PTR)TRUE;
        }
        case IDvsUP:
        {
            if (data->moveY > 0)--(data->moveY);
            data->ifCreate = FALSE;
            RECT rect = { 0,0,data->tableX * data->cellSize,data->tableY * data->cellSize };
            InvalidateRect(hDlg, &rect, TRUE);
            return (INT_PTR)TRUE;
        }
        case IDvsDOWN:
        {
            ++(data->moveY);
            data->ifCreate = FALSE;
            RECT rect = { 0,0,data->tableX * data->cellSize,data->tableY * data->cellSize };
            InvalidateRect(hDlg, &rect, TRUE);
            return (INT_PTR)TRUE;
        }
        case IDvsLEFT:
        {
            if (data->moveX > 0)--(data->moveX);
            data->ifCreate = FALSE;
            RECT rect = { 0,0,data->tableX * data->cellSize,data->tableY * data->cellSize };
            InvalidateRect(hDlg, &rect, TRUE);
            return (INT_PTR)TRUE;
        }
        case IDvsRIGHT:
        {
            ++(data->moveX);
            data->ifCreate = FALSE;
            RECT rect = { 0,0,data->tableX * data->cellSize,data->tableY * data->cellSize };
            InvalidateRect(hDlg, &rect, TRUE);
            return (INT_PTR)TRUE;
        }
        case IDC_CHARA:
        {
            HWND hChara = GetDlgItem(hDlg, IDC_CHARA);
            int index = SendMessage(hChara, CB_GETCURSEL, 0, 0);
            if (index == 0) {
                EnableWindow(GetDlgItem(hDlg, ID_ADDATT), TRUE);
                EnableWindow(GetDlgItem(hDlg, ID_ADDDEF), TRUE);
                EnableWindow(GetDlgItem(hDlg, ID_ADDMUV), TRUE);
                EnableWindow(GetDlgItem(hDlg, ID_ADDSUV), TRUE);
                EnableWindow(GetDlgItem(hDlg, ID_SUBATT), TRUE);
                EnableWindow(GetDlgItem(hDlg, ID_SUBDEF), TRUE);
                EnableWindow(GetDlgItem(hDlg, ID_SUBMUV), TRUE);
                EnableWindow(GetDlgItem(hDlg, ID_SUBSUV), TRUE);
            }
            else {
                EnableWindow(GetDlgItem(hDlg, ID_ADDATT), FALSE);
                EnableWindow(GetDlgItem(hDlg, ID_ADDDEF), FALSE);
                EnableWindow(GetDlgItem(hDlg, ID_ADDMUV), FALSE);
                EnableWindow(GetDlgItem(hDlg, ID_ADDSUV), FALSE);
                EnableWindow(GetDlgItem(hDlg, ID_SUBATT), FALSE);
                EnableWindow(GetDlgItem(hDlg, ID_SUBDEF), FALSE);
                EnableWindow(GetDlgItem(hDlg, ID_SUBMUV), FALSE);
                EnableWindow(GetDlgItem(hDlg, ID_SUBSUV), FALSE);
            }
            SetWindowText(GetDlgItem(hDlg, ID_OUTPUT1), std::to_wstring(data->att[index]).c_str());
            SetWindowText(GetDlgItem(hDlg, ID_OUTPUT2), std::to_wstring(data->def[index]).c_str());
            SetWindowText(GetDlgItem(hDlg, ID_OUTPUT3), std::to_wstring(data->muv[index]).c_str());
            SetWindowText(GetDlgItem(hDlg, ID_OUTPUT4), std::to_wstring(data->suv[index]).c_str());
            RECT rect = { data->colorBlockX,data->colorBlockY,
            data->colorBlockX + data->colorBlockSize,
            data->colorBlockY + data->colorBlockSize };
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
                INT mx = x + data->moveX;
                INT my = y + data->moveY;
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
        HWND hChara = GetDlgItem(hDlg, IDC_CHARA);
        int index = SendMessage(hChara, CB_GETCURSEL, 0, 0); // 获取选中索引
        if (x < data->tableX && y < data->tableY&& index==0) {
            exchangeLife(data->grid[index], x + data->moveX, y + data->moveY);
            RECT rect = { x * data->cellSize, y * data->cellSize,
                        (x + 1) * data->cellSize, (y + 1) * data->cellSize };
            InvalidateRect(hDlg, &rect, TRUE);  // 仅重绘该区域
            data->lastX = x; data->lastY = y;   // 记录上次处理的格子，避免 `WM_MOUSEMOVE` 立即重复处理
        }
        else if (LOWORD(lParam) >= data->colorBlockX 
            && LOWORD(lParam) < data->colorBlockX + data->colorBlockSize
            && HIWORD(lParam) >= data->colorBlockY
            && HIWORD(lParam) < data->colorBlockY + data->colorBlockSize) {
            data->playerColor[index] = DialogBoxParam(GetModuleHandle(NULL),
                MAKEINTRESOURCE(IDD_COLOR), hDlg, color, (LPARAM)(data->playerColor[index]));
            {
                RECT rect = { data->colorBlockX,data->colorBlockY,
                data->colorBlockX + data->colorBlockSize,
                data->colorBlockY + data->colorBlockSize };
                InvalidateRect(hDlg, &rect, TRUE);
            }
            {
                RECT rect = { 0,0,data->tableX * data->cellSize,data->tableY * data->cellSize };
                InvalidateRect(hDlg, &rect, TRUE);
            }
            data->ifCreate = FALSE;
            data->ifMouseDown = FALSE;

            return (INT_PTR)TRUE;
        }
        return (INT_PTR)TRUE;
    }
    case WM_MOUSEMOVE:
    {
        if (data->ifMouseDown) {  // 仅在鼠标按住时处理
            int x = LOWORD(lParam) / data->cellSize;
            int y = HIWORD(lParam) / data->cellSize;
            HWND hChara = GetDlgItem(hDlg, IDC_CHARA);
            int index = SendMessage(hChara, CB_GETCURSEL, 0, 0);
            if (index==0&&(x < data->tableX && y < data->tableY) && (x != data->lastX || y != data->lastY)) {
                // 只有当鼠标进入新格子时才处理，防止重复
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