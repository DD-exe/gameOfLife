#include "framework.h"
#include "gameOfLife.h"
#define MAX_LOADSTRING 100

// 全局变量
HINSTANCE   hInst;                                // 当前实例
WCHAR       szTitle[MAX_LOADSTRING];              // 标题栏文本
WCHAR       szWindowClass[MAX_LOADSTRING];        // 主窗口类名
BOOL        ifRun;
INT         cellSize;
INT         tableX;
INT         tableY;         // table表示法待重构
BOOL      **table;
BOOL        ifMouseDown;    // 释放鼠标按下状态
INT         lastX;
INT         lastY;
// 函数前向声明
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,_In_opt_ HINSTANCE hPrevInstance,_In_ LPWSTR lpCmdLine,_In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 在此处放置代码。

    // 初始化全局字符串
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_GAMEOFLIFE, szWindowClass, MAX_LOADSTRING);
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
   cellSize = 3;
   tableX = 100;
   tableY = 100;
   table = new BOOL * [tableX];
   for (int i = 0; i < tableX; ++i)table[i] = new BOOL[tableY];// TODO:重构table部分
   ifMouseDown = FALSE;
   lastX = lastY = -1;//TODO:

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
                for (int i = 0; i < tableX; ++i) {
                    for (int j = 0; j < tableY; ++j) {
                        table[i][j] = FALSE;
                    }
                }
                InvalidateRect(hWnd, NULL, TRUE);
                break;
            case ID_SAVE:
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            RECT clientRect;
            GetClientRect(hWnd, &clientRect);  // 获取客户区区域
            INT clientWidth = clientRect.right - clientRect.left;   // 客户区宽度
            INT clientHeight = clientRect.bottom - clientRect.top;  // 客户区高度
            static HWND startBotton = CreateWindow(
                L"BUTTON", L"启动/暂停(O)",
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                clientWidth-100, 0, 100, 30,
                hWnd, (HMENU)ID_START, NULL, NULL
            );
            static HWND stopBotton = CreateWindow(
                L"BUTTON", L"重置(U)",
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                clientWidth - 100, 40, 100, 30,
                hWnd, (HMENU)ID_STOP, NULL, NULL
            );
            for (int y = 0; y <tableY; y++) {
                for (int x = 0; x < tableX; x++) {
                    RECT rect = { x * cellSize, y * cellSize,
                        (x + 1) * cellSize, (y + 1) * cellSize };
                    if (table[y][x]) {
                        HBRUSH hBrush = CreateSolidBrush(RGB(100, 100, 255)); // 蓝色填充
                        FillRect(hdc, &rect, hBrush);
                        DeleteObject(hBrush);
                    }
                    else {
                        HBRUSH hBrush = CreateSolidBrush(RGB(200, 200, 200)); // 蓝色填充
                        FillRect(hdc, &rect, hBrush);
                        DeleteObject(hBrush);
                    }
                    /*
                    HPEN hPen = CreatePen(PS_SOLID, 1, RGB(100, 100, 100));
                    HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
                    Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom);                    
                    SelectObject(hdc, hOldPen);
                    DeleteObject(hPen);// 恢复旧画笔
                    */
                }
            }
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_LBUTTONDOWN:
    {
        ifMouseDown = true;  // 标记鼠标按下
        int x = LOWORD(lParam) / cellSize;
        int y = HIWORD(lParam) / cellSize;

        if (x < tableX && y < tableY) {
            table[y][x] = !table[y][x];  // 切换状态
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
                table[y][x] = !table[y][x];
                RECT rect = { x * cellSize, y * cellSize, (x + 1) * cellSize, (y + 1) * cellSize };
                InvalidateRect(hWnd, &rect, TRUE);

                lastX = x;
                lastY = y;  // 记录上次处理的格子
            }
        }
    }
        break;
    case WM_LBUTTONUP:
        ifMouseDown = false;  // 释放鼠标按下状态
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