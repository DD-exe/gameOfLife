#pragma once
#include "targetver.h"
#define WIN32_LEAN_AND_MEAN // 从Windows头文件中排除极少使用的内容
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
#include <fstream>
#include <shlobj.h>
#include <nlohmann/json.hpp>
// stl
#include <unordered_map>
#include <string>

#define RGBgrey     RGB(200,200,200)
#define RGBblack    RGB(0,0,0)
#define RGBwhite    RGB(255,255,255)
#define RGBgreen    RGB(60,255,176)
#define RGBpurple   RGB(180,150,255)
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
    COLORREF    playerColor;
    INT         colorBlockX;
    INT         colorBlockY;
    INT         colorBlockSize;
};
struct vsData {
    std::unordered_map<INT, std::unordered_map<INT, BOOL>> grid[2];
    BOOL ifCreate;
    BOOL ifRun;
    BOOL ifMouseDown;
    INT lastX, lastY;
    INT speed;
    INT cellSize; INT tableX; INT tableY; INT listHalfSize;
    INT4 rule[2];
    INT att[2]; INT def[2];
    INT moveX, moveY;
    COLORREF playerColor[2];
    INT colorBlockX;
    INT colorBlockY;
    INT colorBlockSize;
};
struct coData {
    HWND hParent;
    INT r, g, b;
    COLORREF oc;
    BOOL ifMouseDown;
};
struct vsoData {
    std::unordered_map<INT, std::unordered_map<INT, BOOL>> grid[2];
    BOOL ifCreate;
    BOOL ifServer,ifClient;
    wchar_t targetIP[100];
    nlohmann::json theMove;
    BOOL ifMouseDown;
    INT score[2];       // 设想：开局20点，每回合增加10点，
                        // 1点可改变一格状态，
                        // 2点可增加1攻击或1防御，
                        // 100点可提升1级繁殖或适应，3级为满。
    INT lastX, lastY;
    INT cellSize; INT tableX; INT tableY; INT listHalfSize;
    INT4 rule[2];
    INT att[2]; INT def[2];
    INT muv[2]; INT suv[2];
    INT moveX, moveY;
    COLORREF playerColor[2];
    INT colorBlockX;
    INT colorBlockY;
    INT colorBlockSize;
};
using GridType = std::unordered_map<INT, std::unordered_map<INT, BOOL>>;
BOOL findLife(std::unordered_map<INT, std::unordered_map<INT, BOOL>> &grid, INT x, INT y);      // life
void exchangeLife(std::unordered_map<INT, std::unordered_map<INT, BOOL>>& grid, INT x, INT y);
void myLife(std::unordered_map<INT, std::unordered_map<INT, BOOL>>& grid,
    std::unordered_map<INT, std::unordered_map<INT, BOOL>>& ans,
    INT4& rule, STATE** state);
// 获取用户文档目录下的应用子目录
std::wstring getDefaultSaveDirectory();                                                         // save
bool saveGrid(HWND hWnd, 
    const std::unordered_map<INT, std::unordered_map<INT, BOOL>>& grid);                        
bool saveVSGrid(HWND hWnd, 
    const std::unordered_map<INT, std::unordered_map<INT, BOOL>> grid[2]);
std::unordered_map<INT, std::unordered_map<INT, BOOL>> loadGrid(HWND hWnd);                     // load
void loadVSGrid(HWND hWnd, std::unordered_map<INT, std::unordered_map<INT, BOOL>> grid[2]);
void saveBmp(HWND hWnd, INT x, INT y, INT dx, INT dy);                                          // downloads  
BOOL ofnRead(HWND hWnd, OPENFILENAME& ofn, WCHAR* szFile, DWORD bufSize);
void bfhWrite(BITMAPFILEHEADER& bfh, FILE* file, INT dx, INT dy, INT duiqiX);
void bihWrite(BITMAPINFOHEADER& bih, FILE* file, INT dx, INT dy, INT duiqiX);
INT getAll(std::unordered_map<INT, std::unordered_map<INT, BOOL>>& grid,INT x, INT y, INT z);   // tools
void delState(STATE** x);
int getRandomNum(int min, int max);
std::string wc2s(const wchar_t* wstr);
nlohmann::json move2json(vsoData* data);
void json2move(vsoData* data, nlohmann::json change);
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);                  // about
INT_PTR CALLBACK VSdot(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);                  // vs
INT_PTR CALLBACK single(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);                 // single
void SetSingleWindows(HWND hDlg, siData* data);
void moveSingleWindows(HWND hDlg, siData* data, INT clientWidth);
INT_PTR CALLBACK VSOnline(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);               // vsOnline
INT_PTR CALLBACK VSOnlineDot(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK color(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);                  // color
INT_PTR CALLBACK CreateRoomProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);         // createRoom
INT_PTR CALLBACK JoinRoomProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);           // joinRoom
BOOL runServer(vsoData& data);                                                                 // trans
BOOL runClient(vsoData& data);
void mySendMessage();

// inline
inline void getClientXY(HWND hWnd, INT* x, INT* y) {
    RECT clientRect;
    GetClientRect(hWnd, &clientRect);           // 获取客户区区域
    *x = clientRect.right - clientRect.left;    // 客户区宽度
    *y = clientRect.bottom - clientRect.top;    // 客户区高度
}