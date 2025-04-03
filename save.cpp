/*
    用于实现内置的模拟结果保存功能
*/
#include "framework.h"
#include "gameOfLife.h"
#include <windows.h>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <nlohmann/json.hpp>
#include <shlobj.h> // 用于获取文档目录

using json = nlohmann::json;

// 将 grid 转换为 JSON
static json serializeGrid(const std::unordered_map<INT, std::unordered_map<INT, BOOL>>& grid) {
    json j;
    for (const auto& [y, row] : grid) {
        for (const auto& [x, value] : row) {
            j[std::to_string(y)][std::to_string(x)] = value;
        }
    }
    return j;
}

static json serializeVSGrid(const std::unordered_map<INT, std::unordered_map<INT, BOOL>> grid[2]) {
    json j;

    // 序列化第一个网格 (grid[0])
    json grid0;
    for (const auto& [y, row] : grid[0]) {
        for (const auto& [x, value] : row) {
            grid0[std::to_string(y)][std::to_string(x)] = value;
        }
    }
    j["grid0"] = grid0;

    // 序列化第二个网格 (grid[1])
    json grid1;
    for (const auto& [y, row] : grid[1]) {
        for (const auto& [x, value] : row) {
            grid1[std::to_string(y)][std::to_string(x)] = value;
        }
    }
    j["grid1"] = grid1;

    return j;
}

// 生成默认时间戳文件名
std::wstring generateDefaultFilename() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);

    std::wstringstream wss;
    tm tm_buf;
    localtime_s(&tm_buf, &in_time_t);
    wss << std::put_time(&tm_buf, L"grid_%Y%m%d_%H%M%S.json");

    return wss.str();
}

// 获取默认保存目录（文档/GameOfLifeSaves/）
std::wstring getDefaultSaveDirectory() {
    PWSTR docsPath = nullptr;
    std::wstring saveDir;

    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Documents, 0, nullptr, &docsPath))) {
        saveDir = std::wstring(docsPath) + L"\\GameOfLifeSaves\\";
        CoTaskMemFree(docsPath);
    }
    else {
        saveDir = L".\\saves\\";
    }

    CreateDirectory(saveDir.c_str(), nullptr);
    return saveDir;
}

// 显示保存对话框，带默认文件名
std::wstring showSaveDialog(HWND hWnd, const std::wstring& defaultFilename) {
    OPENFILENAME ofn = { 0 };
    WCHAR szFile[MAX_PATH] = { 0 };

    // 设置默认路径和文件名
    std::wstring defaultPath = getDefaultSaveDirectory() + defaultFilename;
    wcscpy_s(szFile, defaultPath.c_str());

    // 配置对话框
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hWnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = L"JSON Files (*.json)\0*.json\0All Files (*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrDefExt = L"json";
    ofn.lpstrTitle = L"保存网格数据";
    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;

    if (GetSaveFileName(&ofn)) {
        return szFile;
    }

    return L""; // 用户取消
}

// 保存JSON到指定路径
bool saveJsonToFile(const json& j, const std::wstring& filepath) {
    try {
        std::ofstream outfile(filepath, std::ios::binary);
        if (!outfile.is_open()) {
            return false;
        }

        outfile << j.dump(4); // 美化输出，4空格缩进
        return true;
    }
    catch (...) {
        return false;
    }
}

// single保存函数
bool saveGrid(HWND hWnd, const std::unordered_map<INT, std::unordered_map<INT, BOOL>>& grid) {
    // 序列化数据
    json gridJson = serializeGrid(grid); // 使用之前定义的serializeGrid

    // 准备默认文件名
    std::wstring defaultName = generateDefaultFilename();

    // 显示保存对话框
    std::wstring savePath = showSaveDialog(hWnd, defaultName);
    if (savePath.empty()) {
        return false; // 用户取消
    }

    // 保存文件
    if (!saveJsonToFile(gridJson, savePath)) {
        MessageBox(hWnd, L"保存文件失败", L"错误", MB_ICONERROR);
        return false;
    }

    // 显示成功消息
    std::wstring successMsg = L"网格已保存到:\n" + savePath;
    MessageBox(hWnd, successMsg.c_str(), L"保存成功", MB_ICONINFORMATION);

    return true;
}

// vs保存函数
bool saveVSGrid(HWND hWnd, const std::unordered_map<INT, std::unordered_map<INT, BOOL>> grid[2]) {
    // 序列化数据
    json gridJson = serializeVSGrid(grid); // 使用之前定义的serializeGrid

    // 准备默认文件名
    std::wstring defaultName = generateDefaultFilename();

    // 显示保存对话框
    std::wstring savePath = showSaveDialog(hWnd, defaultName);
    if (savePath.empty()) {
        return false; // 用户取消
    }

    // 保存文件
    if (!saveJsonToFile(gridJson, savePath)) {
        MessageBox(hWnd, L"保存文件失败", L"错误", MB_ICONERROR);
        return false;
    }

    // 显示成功消息
    std::wstring successMsg = L"网格已保存到:\n" + savePath;
    MessageBox(hWnd, successMsg.c_str(), L"保存成功", MB_ICONINFORMATION);

    return true;
}