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
HWND        singleBotton, VSBotton, VSButtonA, VSButtonB, doingInfo,
            cellsizeEdit, cellsizeBotton, cellsizeInfo, cellsizeTitle,
            timeEdit, timeBotton, timeInfo, timeTitle,
            modXuTitle, modXuEditX, modXuEditY, modXuBotton, modXuInfo,
            modReTitle, modReEditX, modReEditY, modReBotton, modReInfo;
static HWND hBmpStatic;
INT         listHalfSize;   // 控制栏半宽度
INT         listUnitHeight;
INT         titleSize;
BOOL        VSbuttonsVisible = FALSE;  //对抗按钮可见性

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
    INT clientWidth, clientHeight;
    getClientXY(hWnd, &clientWidth, &clientHeight);
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
            //功能菜单下内容
            case ID_SINGLE:                    // 单机模式
            {
                HWND SingleDialog = CreateDialog(hInst, MAKEINTRESOURCE(IDD_SINGLE), hWnd, single);
                ShowWindow(SingleDialog, SW_SHOW);
            }
            break;
            case ID_VSMODE:                    // 联机模式
            {                
                if (!VSbuttonsVisible) {
                    // 显示按钮 A 和 B
                    ShowWindow(VSButtonA, SW_SHOW);
                    ShowWindow(VSButtonB, SW_SHOW);
                    VSbuttonsVisible = TRUE; // 更新状态
                }
                else {
                    ShowWindow(VSButtonA, SW_HIDE);
                    ShowWindow(VSButtonB, SW_HIDE);
                    VSbuttonsVisible = FALSE;
                }
            }
            break;
            case ID_VS:
            {
                HWND VSOnlineDialog = CreateDialog(hInst, MAKEINTRESOURCE(IDD_VS), hWnd, VSdot);
                ShowWindow(VSOnlineDialog, SW_SHOW);
            }
            break;
            case ID_VSOnline:
            {
                HWND VSDialog = CreateDialog(hInst, MAKEINTRESOURCE(IDD_VSONLINE), hWnd, VSOnlineDot);
                // HWND VSDialog = CreateDialog(hInst, MAKEINTRESOURCE(IDD_VSONLINESET), hWnd, VSOnline);
                ShowWindow(VSDialog, SW_SHOW);
            }
            break;
            case IDM_EXIT:
                //退出
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
            break;
        }
        break;
    case WM_CREATE:
    {
        //创建右侧菜单栏
        INT clientWidth,clientHeight;
        getClientXY(hWnd, &clientWidth, &clientHeight);
        // 插入一系列控件
        singleBotton = CreateWindow(
            L"BUTTON", L"单机模式",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            clientWidth / 2 - 50, clientHeight * 3 / 5, 100, 30,
            hWnd, (HMENU)ID_SINGLE, NULL, NULL
        );
        VSBotton = CreateWindow(
            L"BUTTON", L"对抗模式",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            clientWidth / 2 - 50, clientHeight * 3 / 5 + 40, 100, 30,
            hWnd, (HMENU)ID_VSMODE, NULL, NULL
        );
        VSButtonA = CreateWindow(
            L"BUTTON", L"本地对抗",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            clientWidth / 2 - 50, clientHeight * 3 / 5 + 80, 100, 30,
            hWnd, (HMENU)ID_VS, NULL, NULL
        );

        VSButtonB = CreateWindow(
            L"BUTTON", L"联机对抗",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            clientWidth / 2 - 50, clientHeight * 3 / 5 + 120, 100, 30,
            hWnd, (HMENU)ID_VSOnline, NULL, NULL
        );
        ShowWindow(VSButtonA, SW_HIDE);
        ShowWindow(VSButtonB, SW_HIDE);
        VSbuttonsVisible = FALSE;       
    }
        break;
    case WM_SIZE:
    {
        int clientWidth = LOWORD(lParam);
        int clientHeight = HIWORD(lParam);
        tableX = (clientWidth - 2 * listHalfSize - 10) / cellSize;
        tableY = clientHeight / cellSize;
        ifCreate = FALSE;
        MoveWindow(singleBotton, clientWidth / 2 - 50, clientHeight * 3 / 5, 100, 30, TRUE);
        MoveWindow(VSBotton, clientWidth / 2 - 50, clientHeight * 3 / 5 + 40, 100, 30, TRUE);
        MoveWindow(VSButtonA, clientWidth / 2 - 50, clientHeight * 3 / 5 + 80, 100, 30, TRUE);
        MoveWindow(VSButtonB, clientWidth / 2 - 50, clientHeight * 3 / 5 + 120, 100, 30, TRUE);

        MoveWindow(hBmpStatic, clientWidth - 2 * listHalfSize,
            listUnitHeight * 6, 2 * listHalfSize, 3 * listHalfSize, TRUE);
    }
        break;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps); // 什么都不画也行，但必须调用这两个函数——留作教训
        Gdiplus::Graphics graphics(hdc);
        Gdiplus::Image image(L"image/main.png");
        graphics.DrawImage(&image, 0, 0, clientWidth, clientHeight);

        EndPaint(hWnd, &ps);
        break;
    }
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