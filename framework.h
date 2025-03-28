﻿#pragma once
#include "targetver.h"
#define WIN32_LEAN_AND_MEAN             // 从Windows头文件中排除极少使用的内容
// Windows 头文件
#include <windows.h>
// C 运行时头文件
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
// 文件系统访问
#include <commdlg.h>
#include <stdio.h>
// stl
#include <unordered_map>
#include <string>

enum STATE {
    DEAD, LIVE
};
extern STATE** state;
struct INT4 {
    int x, y, z, t;
};
struct siData {
    // 表格问题
    BOOL        ifRun;
    BOOL        ifCreate;
    INT         cellSize;
    INT         speed;
    INT         tableX;
    INT         tableY;
    std::unordered_map<INT, std::unordered_map<INT, BOOL>> grid;
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
    HWND        hBmpStatic;
    INT         listHalfSize;   // 控制栏半宽度
    INT         listUnitHeight;
    INT         titleSize;
};
struct vsData{
    std::unordered_map<INT, std::unordered_map<INT, BOOL>> gridP1;
    std::unordered_map<INT, std::unordered_map<INT, BOOL>> gridP2;
    BOOL ifCreate;
    BOOL ifRun;
    BOOL ifMouseDown;
    INT lastX, lastY;
    INT speed;
    INT cellSize; INT tableX; INT tableY; INT listHalfSize;
    INT4 ruleP1; INT4 ruleP2;
    INT attP1; INT defP1;
    INT attP2; INT defP2;
    INT moveX, moveY;
};
BOOL findLife(std::unordered_map<INT, std::unordered_map<INT, BOOL>> &grid, INT x, INT y);      // life
void exchangeLife(std::unordered_map<INT, std::unordered_map<INT, BOOL>>& grid, INT x, INT y);
void myLife(std::unordered_map<INT, std::unordered_map<INT, BOOL>>& grid,
    std::unordered_map<INT, std::unordered_map<INT, BOOL>>& ans,
    INT4& rule, STATE** state);
void saveBmp(HWND hWnd, INT x, INT y, INT dx, INT dy);                                          //downloads
BOOL ofnRead(HWND hWnd, OPENFILENAME& ofn, WCHAR* szFile, DWORD bufSize);
void bfhWrite(BITMAPFILEHEADER& bfh, FILE* file, INT dx, INT dy, INT duiqiX);
void bihWrite(BITMAPINFOHEADER& bfh, FILE* file, INT dx, INT dy, INT duiqiX);
INT getAll(std::unordered_map<INT, std::unordered_map<INT, BOOL>>& grid,INT x, INT y, INT z);   // tools
void delState(STATE** x);
int getRandomNum(int min, int max);
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);                  // about
INT_PTR CALLBACK VSdot(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);                  // vs
INT_PTR CALLBACK single(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);                 // single
void SetSingleWindows(HWND hDlg, siData* data);
void moveSingleWindows(HWND hDlg, siData* data, INT clientWidth);

// inline
inline void getClientXY(HWND hWnd, INT* x, INT* y) {
    RECT clientRect;
    GetClientRect(hWnd, &clientRect);           // 获取客户区区域
    *x = clientRect.right - clientRect.left;    // 客户区宽度
    *y = clientRect.bottom - clientRect.top;    // 客户区高度
}