/*
    程序主框架
    用于创建主窗口和维护消息循环
*/
#include "framework.h"
#include "gameOfLife.h"
#define MAX_LOADSTRING 100

#define RGBgrey     RGB(200,200,200)
#define RGBwhite    RGB(255,255,255)
#define RGBblue     RGB(100,100,255)
#define RGBpurple   RGB(180,150,255)
// 全局变量
HINSTANCE   hInst;                                // 当前实例
WCHAR       szTitle[MAX_LOADSTRING];              // 标题栏文本
WCHAR       szWindowClass[MAX_LOADSTRING];        // 主窗口类名
// 表格问题
BOOL        ifRun;
BOOL        ifCreate;
INT         cellSize;
INT         speed;
INT         tableX;
INT         tableY;         
std::unordered_map<INT, std::unordered_map<INT, BOOL>> grid;
STATE**                                                state;
INT4                                                   rule;

ULONG_PTR   gdiplusToken;
// 鼠标问题
BOOL        ifMouseDown;    // 释放鼠标按下状态
INT         lastX;
INT         lastY;
// 控制栏问题
HWND        startBotton, stopBotton, doingInfo,
            cellsizeEdit, cellsizeBotton, cellsizeInfo, cellsizeTitle,
            timeEdit, timeBotton, timeInfo, timeTitle,
            modXuTitle, modXuEditX, modXuEditY, modXuBotton, modXuInfo,
            modReTitle, modReEditX, modReEditY, modReBotton, modReInfo;
static HWND hBmpStatic;
INT         listHalfSize;   // 控制栏半宽度
INT         listUnitHeight;
INT         titleSize;
// 函数前向声明
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,_In_opt_ HINSTANCE hPrevInstance,_In_ LPWSTR lpCmdLine,_In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    // 初始化全局字符串
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_GAMEOFLIFE, szWindowClass, MAX_LOADSTRING);
    // 初始化gdi+
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
    MyRegisterClass(hInstance);
    if (!InitInstance (hInstance, nCmdShow))return FALSE; // 执行应用程序初始化

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_GAMEOFLIFE));

    MSG msg;

    // 主消息循环:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    Gdiplus::GdiplusShutdown(gdiplusToken);
    return (int) msg.wParam;
}

// 注册窗口类。
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_LINico));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_GAMEOFLIFE);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_LINico));

    return RegisterClassExW(&wcex);
}

// 保存实例句柄，创建主窗口
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 将实例句柄存储在全局变量中
   ifRun = FALSE;
   cellSize = 10;
   state = new STATE * [9 * 9 * 9 * 9];
   for (int i = 0; i < 9 * 9 * 9 * 9; ++i)state[i] = nullptr;
   rule.x = 2; rule.y = 3; rule.z = 3; rule.t = 3;
   speed = 10;
   ifCreate = FALSE;
   ifMouseDown = FALSE;
   lastX = lastY = -1;//TODO: done
   listHalfSize = 150;
   listUnitHeight = 40;
   titleSize = 80;

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

// 处理主窗口消息。
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 菜单选择
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case ID_START:
            {
                ifRun = !ifRun;
                if (ifRun) {
                    SetWindowText(doingInfo, L"||");
                }
                else {
                    SetWindowText(doingInfo, L">");
                }
            }                
                break;
            case ID_STOP:
            {
                grid.clear(); ifCreate = FALSE;
                RECT rect = { 0,0,tableX * cellSize,tableY * cellSize };
                InvalidateRect(hWnd, &rect, TRUE);
            }
                break;
            case ID_EDIT1OK:
            {
                BOOL success; ifCreate = FALSE;
                INT x = GetDlgItemInt(hWnd, ID_EDIT1, &success, TRUE);
                INT oX = tableX * cellSize;
                INT oY = tableY * cellSize;
                if (success) { 
                    cellSize = x;
                    INT clientWidth, clientHeight;
                    getClientXY(hWnd, &clientWidth, &clientHeight);
                    tableX = (clientWidth - 2 * listHalfSize - 10) / cellSize;
                    tableY = clientHeight / cellSize;
                    oX = tableX * cellSize > oX ? tableX * cellSize : oX;
                    oY = tableY * cellSize > oY ? tableY * cellSize : oY;
                    RECT rect = { 0,0,oX,oY };
                    // COLORREF it=SetTextColor(GetDC(cellsizeInfo), RGB(255, 255, 0));
                    SetWindowText(cellsizeInfo, std::to_wstring(cellSize).c_str());
                    InvalidateRect(hWnd, &rect, TRUE);
                }
            }                
                break;
            case ID_EDIT2OK:
            {
                BOOL success;
                speed = GetDlgItemInt(hWnd, ID_EDIT2, &success, TRUE);
                if (success) {
                    KillTimer(hWnd, ID_TIMER);
                    SetTimer(hWnd, ID_TIMER, 100 * speed, NULL);
                    SetWindowText(timeInfo, std::to_wstring(speed).c_str());
                }
            }
            break;
            case ID_EDIT3OK:
            {
                BOOL success1,success2;
                INT x = GetDlgItemInt(hWnd, ID_EDIT3X, &success1, TRUE);
                INT y = GetDlgItemInt(hWnd, ID_EDIT3Y, &success2, TRUE);
                if (success1&&success2) {
                    if (x < y) {
                        rule.x = x; rule.y = y;
                        SetWindowText(modXuInfo, (std::to_wstring(x)+L"-"+ std::to_wstring(y)).c_str());
                    }
                    else if (x == y) {
                        rule.x = x; rule.y = y;
                        SetWindowText(modXuInfo, std::to_wstring(x).c_str());
                    }                     
                }
            }
                break;
            case ID_EDIT4OK:
            {
                BOOL success1, success2;
                INT x = GetDlgItemInt(hWnd, ID_EDIT4X, &success1, TRUE);
                INT y = GetDlgItemInt(hWnd, ID_EDIT4Y, &success2, TRUE);
                if (success1 && success2) {
                    if (x < y) {
                        rule.z = x; rule.t = y;
                        SetWindowText(modReInfo, (std::to_wstring(x) + L"-" + std::to_wstring(y)).c_str());
                    }
                    else if (x == y) {
                        rule.z = x; rule.t = y;
                        SetWindowText(modReInfo, std::to_wstring(x).c_str());
                    }
                }
            }
                break;
            case ID_SAVE:
                saveBmp(hWnd, 0, 0, cellSize * tableX, cellSize * tableY);
                break;
            case ID_VS:                     // 单机对战
            {
                HWND neoDialog = CreateDialog(hInst, MAKEINTRESOURCE(IDD_VS), hWnd, VSdot);
                ShowWindow(neoDialog, SW_SHOW);
            }                
                break;
            case ID_SINGLE:                     // 单机模拟
            {
                HWND neoDialog = CreateDialog(hInst, MAKEINTRESOURCE(IDD_SINGLE), hWnd, single);
                ShowWindow(neoDialog, SW_SHOW);
            }
            break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_TIMER:
        if (ifRun && wParam == ID_TIMER) {
            std::unordered_map<INT, std::unordered_map<INT, BOOL>> ans;
            // INT4 rectx;
            myLife(grid, ans, rule, state);
            RECT rect = { 0, 0,tableX* cellSize, tableY* cellSize };
            grid = std::move(ans);
            InvalidateRect(hWnd, &rect, TRUE);
        }
        break;
    case WM_CREATE:
    {
        SetTimer(hWnd, ID_TIMER, 100 * speed, NULL); // 初始化计时器，WM_TIMER需要
        INT clientWidth,clientHeight;
        getClientXY(hWnd, &clientWidth, &clientHeight);
        tableX = (clientWidth - 2 * listHalfSize - 10) / cellSize;
        tableY = clientHeight / cellSize; // 计算右边控制栏位置
        // 插入一系列控件
        startBotton = CreateWindow(
            L"BUTTON", L"启动/暂停(O)",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            clientWidth - 50 - listHalfSize, listUnitHeight * 0, 100, 30,
            hWnd, (HMENU)ID_START, NULL, NULL
        );
        stopBotton = CreateWindow(
            L"BUTTON", L"重置(U)",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            clientWidth - 50 - listHalfSize, listUnitHeight * 1, 100, 30,
            hWnd, (HMENU)ID_STOP, NULL, NULL
        );
        doingInfo = CreateWindow(TEXT("STATIC"), TEXT(">"),
            WS_CHILD | WS_VISIBLE | SS_CENTERIMAGE | SS_CENTER,
            clientWidth - listHalfSize + 50, listUnitHeight * 0, 30, 30,
            hWnd, (HMENU)ID_DOING, hInst, NULL);

        cellsizeTitle = CreateWindow(TEXT("STATIC"), TEXT("设置网格："),
            WS_CHILD | WS_VISIBLE | SS_CENTER,
            clientWidth - 50 - listHalfSize-titleSize, listUnitHeight * 2, titleSize, 30,
            hWnd, (HMENU)ID_TITLE1, hInst, NULL);
        cellsizeEdit = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"10",
            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
            clientWidth - 50 - listHalfSize, listUnitHeight * 2, 50, 30,
            hWnd, (HMENU)ID_EDIT1, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL
        );
        cellsizeBotton = CreateWindow(
            L"BUTTON", L"确认",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            clientWidth - listHalfSize, listUnitHeight * 2, 50, 30,
            hWnd, (HMENU)ID_EDIT1OK, NULL, NULL
        );
        cellsizeInfo = CreateWindow(TEXT("STATIC"), TEXT(""),
            WS_CHILD | WS_VISIBLE | SS_CENTERIMAGE| SS_CENTER,
            clientWidth - listHalfSize + 50, listUnitHeight * 2 , 30, 30,
            hWnd, (HMENU)ID_OUTPUT1, hInst, NULL);
        SetWindowText(cellsizeInfo, std::to_wstring(cellSize).c_str());

        timeTitle = CreateWindow(TEXT("STATIC"), TEXT("迭代用时："),
            WS_CHILD | WS_VISIBLE | SS_CENTER,
            clientWidth - 50 - listHalfSize - titleSize, listUnitHeight * 3, titleSize, 30,
            hWnd, (HMENU)ID_TITLE2, hInst, NULL);
        timeEdit = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"10",
            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
            clientWidth - 50 - listHalfSize, listUnitHeight * 3, 50, 30,
            hWnd, (HMENU)ID_EDIT2, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL
        );
        timeBotton = CreateWindow(
            L"BUTTON", L"确认",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            clientWidth - listHalfSize, listUnitHeight * 3, 50, 30,
            hWnd, (HMENU)ID_EDIT2OK, NULL, NULL
        );
        timeInfo = CreateWindow(TEXT("STATIC"), TEXT(""),
            WS_CHILD | WS_VISIBLE | SS_CENTERIMAGE | SS_CENTER,
            clientWidth - listHalfSize + 50, listUnitHeight * 3, 30, 30,
            hWnd, (HMENU)ID_OUTPUT2, hInst, NULL);
        SetWindowText(timeInfo, std::to_wstring(speed).c_str());

        modXuTitle = CreateWindow(TEXT("STATIC"), TEXT("存活需求："),
            WS_CHILD | WS_VISIBLE | SS_CENTER,
            clientWidth - 50 - listHalfSize - titleSize, listUnitHeight * 4, titleSize, 30,
            hWnd, (HMENU)ID_TITLE3, hInst, NULL);
        modXuEditX = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"2",
            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
            clientWidth - 50 - listHalfSize, listUnitHeight * 4, 25, 30,
            hWnd, (HMENU)ID_EDIT3X, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL
        );
        modXuEditY = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"3",
            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
            clientWidth - 25 - listHalfSize, listUnitHeight * 4, 25, 30,
            hWnd, (HMENU)ID_EDIT3Y, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL
        );
        modXuBotton = CreateWindow(
            L"BUTTON", L"确认",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            clientWidth - listHalfSize, listUnitHeight * 4, 50, 30,
            hWnd, (HMENU)ID_EDIT3OK, NULL, NULL
        );
        modXuInfo = CreateWindow(TEXT("STATIC"), TEXT("2-3"),
            WS_CHILD | WS_VISIBLE | SS_CENTERIMAGE | SS_CENTER,
            clientWidth - listHalfSize + 50, listUnitHeight * 4, 50, 30,
            hWnd, (HMENU)ID_OUTPUT3, hInst, NULL);

        modReTitle = CreateWindow(TEXT("STATIC"), TEXT("繁殖需求："),
            WS_CHILD | WS_VISIBLE | SS_CENTER,
            clientWidth - 50 - listHalfSize - titleSize, listUnitHeight * 5, titleSize, 30,
            hWnd, (HMENU)ID_TITLE4, hInst, NULL);
        modReEditX = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"3",
            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
            clientWidth - 50 - listHalfSize, listUnitHeight * 5, 25, 30,
            hWnd, (HMENU)ID_EDIT4X, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL
        );
        modReEditY = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"3",
            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
            clientWidth - 25 - listHalfSize, listUnitHeight * 5, 25, 30,
            hWnd, (HMENU)ID_EDIT4Y, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL
        );
        modReBotton = CreateWindow(
            L"BUTTON", L"确认",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            clientWidth - listHalfSize, listUnitHeight * 5, 50, 30,
            hWnd, (HMENU)ID_EDIT4OK, NULL, NULL
        );
        modReInfo = CreateWindow(TEXT("STATIC"), TEXT("3"),
            WS_CHILD | WS_VISIBLE | SS_CENTERIMAGE | SS_CENTER,
            clientWidth - listHalfSize + 50, listUnitHeight * 5, 50, 30,
            hWnd, (HMENU)ID_OUTPUT4, hInst, NULL);

        HBITMAP hBmp = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_LINbmpPro));
        hBmpStatic = CreateWindow(
            L"STATIC", NULL,
            WS_CHILD | WS_VISIBLE | SS_BITMAP,
            clientWidth - 2 * listHalfSize, listUnitHeight * 6, 2 * listHalfSize, 3 * listHalfSize,
            hWnd, NULL, NULL, NULL
        );
        SendMessage(hBmpStatic, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hBmp);// 关联 BMP 图片到静态控件       
    }
        break;
    case WM_SIZE:
    {
        int clientWidth = LOWORD(lParam);
        int clientHeight = HIWORD(lParam);
        tableX = (clientWidth - 2 * listHalfSize - 10) / cellSize;
        tableY = clientHeight / cellSize;
        ifCreate = FALSE;
        MoveWindow(startBotton, clientWidth - 50 - listHalfSize,
            listUnitHeight * 0, 100, 30, TRUE);
        MoveWindow(stopBotton, clientWidth - 50 - listHalfSize,
            listUnitHeight * 1, 100, 30, TRUE);
        MoveWindow(doingInfo, clientWidth - listHalfSize + 50,
            listUnitHeight * 0, 30, 30, TRUE);

        MoveWindow(cellsizeTitle, clientWidth - 50 - listHalfSize - titleSize,
            listUnitHeight * 2, titleSize, 30, TRUE);
        MoveWindow(cellsizeEdit, clientWidth - 50 - listHalfSize,
            listUnitHeight * 2, 50, 30, TRUE);
        MoveWindow(cellsizeBotton, clientWidth - listHalfSize,
            listUnitHeight * 2, 50, 30, TRUE);
        MoveWindow(cellsizeInfo, clientWidth - listHalfSize + 50,
            listUnitHeight * 2, 30, 30, TRUE);

        MoveWindow(timeTitle, clientWidth - 50 - listHalfSize - titleSize,
            listUnitHeight * 3, titleSize, 30, TRUE);
        MoveWindow(timeEdit, clientWidth - 50 - listHalfSize,
            listUnitHeight * 3, 50, 30, TRUE);
        MoveWindow(timeBotton, clientWidth - listHalfSize,
            listUnitHeight * 3, 50, 30, TRUE);
        MoveWindow(timeInfo, clientWidth - listHalfSize + 50,
            listUnitHeight * 3, 30, 30, TRUE);

        MoveWindow(modXuTitle, clientWidth - 50 - listHalfSize - titleSize,
            listUnitHeight * 4, titleSize, 30, TRUE);
        MoveWindow(modXuEditX, clientWidth - 50 - listHalfSize,
            listUnitHeight * 4, 25, 30, TRUE);
        MoveWindow(modXuEditY, clientWidth - 25 - listHalfSize,
            listUnitHeight * 4, 25, 30, TRUE);
        MoveWindow(modXuBotton, clientWidth - listHalfSize,
            listUnitHeight * 4, 50, 30, TRUE);
        MoveWindow(modXuInfo, clientWidth - listHalfSize + 50, 
            listUnitHeight * 4, 50, 30, TRUE);

        MoveWindow(modReTitle, clientWidth - 50 - listHalfSize - titleSize,
            listUnitHeight * 5, titleSize, 30, TRUE);
        MoveWindow(modReEditX, clientWidth - 50 - listHalfSize,
            listUnitHeight * 5, 25, 30, TRUE);
        MoveWindow(modReEditY, clientWidth - 25 - listHalfSize,
            listUnitHeight * 5, 25, 30, TRUE);
        MoveWindow(modReBotton, clientWidth - listHalfSize,
            listUnitHeight * 5, 50, 30, TRUE);
        MoveWindow(modReInfo, clientWidth - listHalfSize + 50,
            listUnitHeight * 5, 50, 30, TRUE);

        MoveWindow(hBmpStatic, clientWidth - 2 * listHalfSize,
            listUnitHeight * 6, 2 * listHalfSize, 3 * listHalfSize, TRUE);
    }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            HBRUSH hBrushLive = CreateSolidBrush(RGBpurple);
            HBRUSH hBrushDead = CreateSolidBrush(RGBwhite);
            RECT rectFull = { 0,0,tableX * cellSize, tableY * cellSize };
            FillRect(hdc, &rectFull, hBrushDead);
            for (int y = 0; y <tableY; y++) {
                for (int x = 0; x < tableX; x++) {
                    RECT rect = { x * cellSize, y * cellSize,
                        (x + 1) * cellSize, (y + 1) * cellSize };
                    if (findLife(grid,x,y)) FillRect(hdc, &rect, hBrushLive);                                   
                }
            }
            if (!ifCreate) {
                Gdiplus::Graphics graphics(hdc);
                myPaintFrame(graphics, 0, 0, tableX * cellSize, tableY * cellSize, cellSize);
                ifCreate = TRUE;
            }            
            DeleteObject(hBrushLive);
            DeleteObject(hBrushDead);
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_LBUTTONDOWN:
    {
        ifMouseDown = TRUE;  // 标记鼠标按下
        int x = LOWORD(lParam) / cellSize;
        int y = HIWORD(lParam) / cellSize;

        if (x < tableX && y < tableY) {
            exchangeLife(grid, x, y);
            RECT rect = { x * cellSize, y * cellSize,
                        (x + 1) * cellSize, (y + 1) * cellSize };
            InvalidateRect(hWnd, &rect, TRUE);  // 仅重绘该区域
            lastX = x; lastY = y;  // 记录上次处理的格子，避免 `WM_MOUSEMOVE` 立即重复处理
        }
    }
        break;
    case WM_MOUSEMOVE:
    {
        if (ifMouseDown) {  // 仅在鼠标按住时处理
            int x = LOWORD(lParam) / cellSize;
            int y = HIWORD(lParam) / cellSize;

            if (x < tableX && y < tableY && (x != lastX || y != lastY)) {
                // 只有当鼠标进入新格子时才处理，防止重复
                exchangeLife(grid, x, y);
                RECT rect = { x * cellSize, y * cellSize, (x + 1) * cellSize, (y + 1) * cellSize };
                InvalidateRect(hWnd, &rect, TRUE);

                lastX = x;
                lastY = y;  // 记录上次处理的格子
            }
        }
    }
        break;
    case WM_LBUTTONUP:
        ifMouseDown = FALSE;  // 释放鼠标按下状态
        lastX = lastY = -1;   // 清除上次处理的格子记录
        break;
    case WM_KEYDOWN:
        if (wParam == VK_RETURN) {
            SendMessage(GetDlgItem(hWnd, ID_EDIT1OK), BM_CLICK, 0, 0);
            SendMessage(GetDlgItem(hWnd, ID_EDIT2OK), BM_CLICK, 0, 0);
        }
        break;
    case WM_DESTROY:
        KillTimer(hWnd, ID_TIMER);
        delState(state);
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}