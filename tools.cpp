#include "framework.h"
#include "gameOfLife.h"

void getClientXY(HWND hWnd,INT* x,INT* y) {
    RECT clientRect;
    GetClientRect(hWnd, &clientRect);  // 获取客户区区域
    *x = clientRect.right - clientRect.left;   // 客户区宽度
    *y = clientRect.bottom - clientRect.top;  // 客户区高度
}