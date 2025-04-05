#include "framework.h"
#include "gameOfLife.h"
#include <windows.h>
#include <fstream>
#include <nlohmann/json.hpp>
#include <shlobj.h>

using json = nlohmann::json;

// 从文件读取JSON数据
static json loadJsonFromFile(const std::wstring& filepath) {
    try {
        std::ifstream infile(filepath);
        if (!infile.is_open()) {
            throw std::runtime_error("无法打开文件");
        }

        json j;
        infile >> j;
        return j;
    }
    catch (const std::exception& e) {
        throw std::runtime_error(std::string("读取JSON失败: ") + e.what());
    }
}

// 反序列化JSON到std::unordered_map<INT, std::unordered_map<INT, BOOL>>
static std::unordered_map<INT, std::unordered_map<INT, BOOL>> deserializeGrid(const json& j) {
    std::unordered_map<INT, std::unordered_map<INT, BOOL>> grid;

    try {
        for (auto& [yStr, row] : j.items()) {
            INT y = std::stoi(yStr);
            for (auto& [xStr, value] : row.items()) {
                INT x = std::stoi(xStr);
                grid[y][x] = value.get<BOOL>();
            }
        }
    }
    catch (const std::exception& e) {
        throw std::runtime_error(std::string("解析网格数据失败: ") + e.what());
    }

    return grid;
}

void deserializeVSGrid(const json& j, std::unordered_map<INT, std::unordered_map<INT, BOOL>> grid[2]) {
    // 清空现有数据
    grid[0].clear();
    grid[1].clear();

    // 反序列化第一个网格
    if (j.contains("grid0")) {
        for (const auto& [yStr, row] : j["grid0"].items()) {
            INT y = std::stoi(yStr);
            for (const auto& [xStr, value] : row.items()) {
                INT x = std::stoi(xStr);
                grid[0][y][x] = value.get<BOOL>();
            }
        }
    }

    // 反序列化第二个网格
    if (j.contains("grid1")) {
        for (const auto& [yStr, row] : j["grid1"].items()) {
            INT y = std::stoi(yStr);
            for (const auto& [xStr, value] : row.items()) {
                INT x = std::stoi(xStr);
                grid[1][y][x] = value.get<BOOL>();
            }
        }
    }
}

// 显示文件选择对话框
std::wstring openFileDialog(HWND hWnd) {
    OPENFILENAME ofn;
    WCHAR szFile[260] = { 0 };

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hWnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile) / sizeof(WCHAR);
    ofn.lpstrFilter = L"JSON Files\0*.json\0All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrTitle = L"选择网格文件";
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileName(&ofn) == TRUE) {
        return std::wstring(szFile);
    }

    return L"";
}

// 主加载函数
std::unordered_map<INT, std::unordered_map<INT, BOOL>> loadGrid(HWND hWnd) {
    // 显示文件选择对话框
    std::wstring filepath = openFileDialog(hWnd);
    if (filepath.empty()) {
        return std::unordered_map<INT, std::unordered_map<INT, BOOL>>();
    }

    try {
        // 读取并解析JSON
        json gridJson = loadJsonFromFile(filepath);
        std::unordered_map<INT, std::unordered_map<INT, BOOL>> grid = deserializeGrid(gridJson);
        return grid;
    }
    catch (const std::exception& e) {
        MessageBoxA(hWnd, e.what(), "加载错误", MB_ICONERROR);
        return std::unordered_map<INT, std::unordered_map<INT, BOOL>>(); // 返回空网格
    }
}

void loadVSGrid(HWND hWnd, std::unordered_map<INT, std::unordered_map<INT, BOOL>> grid[2]) {
    // 显示文件选择对话框
    std::wstring filepath = openFileDialog(hWnd);
    if (filepath.empty()) {
        MessageBox(hWnd, L"用户取消加载", L"NULL", MB_ICONERROR);
        return;
    }

    try {
        std::ifstream infile(filepath);
        if (!infile.is_open()) {
            return;
        }

        json gridJson;
        infile >> gridJson;

        deserializeVSGrid(gridJson, grid);
        return;
    }
    catch (...) {
        MessageBox(hWnd, L"加载文件失败", L"错误", MB_ICONERROR);
        return;
    }
}