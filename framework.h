#pragma once
#include "targetver.h"
#define WIN32_LEAN_AND_MEAN             // 从Windows头文件中排除极少使用的内容
// Windows 头文件
#include <windows.h>
// C 运行时头文件
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

void life(int* that, int* newi); // life
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);// about

// inline
inline void getClientXY(HWND hWnd, INT* x, INT* y) {
    RECT clientRect;
    GetClientRect(hWnd, &clientRect);  // 获取客户区区域
    *x = clientRect.right - clientRect.left;   // 客户区宽度
    *y = clientRect.bottom - clientRect.top;  // 客户区高度
}