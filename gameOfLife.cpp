#include "framework.h"
#include "gameOfLife.h"
#define MAX_LOADSTRING 100

#define RGBgrey     RGB(200,200,200)
#define RGBwhite    RGB(255,255,255)
#define RGBblue     RGB(100,100,255)
// 全局变量
HINSTANCE   hInst;                                // 当前实例
WCHAR       szTitle[MAX_LOADSTRING];              // 标题栏文本
WCHAR       szWindowClass[MAX_LOADSTRING];        // 主窗口类名
// 表格问题
BOOL        ifRun;
BOOL        ifCreate;
INT         cellSize;
INT         tableX;
INT         tableY;         
std::unordered_map<INT, std::unordered_map<INT, BOOL>> grid;

ULONG_PTR   gdiplusToken;
// 鼠标问题
BOOL        ifMouseDown;    // 释放鼠标按下状态
INT         lastX;
INT         lastY;
// 控制栏问题
HWND        startBotton, stopBotton, cellsizeEdit, cellsizeBotton;
static HWND hBmpStatic;
INT         listHalfSize;   // 控制栏半宽度
INT         listUnitHeight;
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
   cellSize = 7;
   ifCreate = FALSE;
   ifMouseDown = FALSE;
   lastX = lastY = -1;//TODO:
   listHalfSize = 150;
   listUnitHeight = 40;

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
                ifRun = !ifRun;
                break;
            case ID_STOP:
                grid.clear();
                InvalidateRect(hWnd, NULL, TRUE);
                break;
            case ID_EDIT1OK:
            {
                BOOL success;
                INT x = GetDlgItemInt(hWnd, ID_EDIT1, &success, TRUE);
                if (success) { 
                    cellSize = x;
                    INT clientWidth, clientHeight;
                    getClientXY(hWnd, &clientWidth, &clientHeight);
                    tableX = (clientWidth - 2 * listHalfSize - 10) / cellSize;
                    tableY = clientHeight / cellSize;
                    InvalidateRect(hWnd, NULL, TRUE);
                }
            }                
                break;
            case ID_SAVE:
                saveBmp(hWnd, 0, 0, cellSize * tableX, cellSize * tableY);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_CREATE:
    {
        INT clientWidth,clientHeight;
        getClientXY(hWnd, &clientWidth, &clientHeight);
        tableX = (clientWidth - 2 * listHalfSize - 10) / cellSize;
        tableY = clientHeight / cellSize;
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

        cellsizeEdit = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"7",
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

        HBITMAP hBmp = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_LINbmpPro));
        hBmpStatic = CreateWindow(
            L"STATIC", NULL,
            WS_CHILD | WS_VISIBLE | SS_BITMAP,
            clientWidth - 2*listHalfSize, listUnitHeight * 4, 2 * listHalfSize, 3 * listHalfSize,
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
        MoveWindow(startBotton, clientWidth - 50 - listHalfSize,
            listUnitHeight * 0, 100, 30, TRUE);
        MoveWindow(stopBotton, clientWidth - 50 - listHalfSize,
            listUnitHeight * 1, 100, 30, TRUE);
        MoveWindow(cellsizeEdit, clientWidth - 50 - listHalfSize,
            listUnitHeight * 2, 50, 30, TRUE);
        MoveWindow(cellsizeBotton, clientWidth - listHalfSize,
            listUnitHeight * 2, 50, 30, TRUE);
        MoveWindow(hBmpStatic, clientWidth - 2 * listHalfSize,
            listUnitHeight * 4, 2 * listHalfSize, 3 * listHalfSize, TRUE);
    }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            HBRUSH hBrushLive = CreateSolidBrush(RGBwhite);
            HBRUSH hBrushDead = CreateSolidBrush(RGBblue);
            for (int y = 0; y <tableY; y++) {
                for (int x = 0; x < tableX; x++) {
                    RECT rect = { x * cellSize, y * cellSize,
                        (x + 1) * cellSize, (y + 1) * cellSize };
                    if (findLife(grid,x,y)) FillRect(hdc, &rect, hBrushLive);
                    else FillRect(hdc, &rect, hBrushDead);                                     
                }
            }
            Gdiplus::Graphics graphics(hdc);
            myPaintFrame(graphics, 0, 0, tableX * cellSize, tableY * cellSize, cellSize);
            
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
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}