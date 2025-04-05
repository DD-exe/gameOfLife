#include "framework.h"
#include "gameOfLife.h"
#include <windows.h>
#include <fstream>
#include <nlohmann/json.hpp>
#include <shlobj.h>

using json = nlohmann::json;

// ���ļ���ȡJSON����
static json loadJsonFromFile(const std::wstring& filepath) {
    try {
        std::ifstream infile(filepath);
        if (!infile.is_open()) {
            throw std::runtime_error("�޷����ļ�");
        }

        json j;
        infile >> j;
        return j;
    }
    catch (const std::exception& e) {
        throw std::runtime_error(std::string("��ȡJSONʧ��: ") + e.what());
    }
}

// �����л�JSON��std::unordered_map<INT, std::unordered_map<INT, BOOL>>
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
        throw std::runtime_error(std::string("������������ʧ��: ") + e.what());
    }

    return grid;
}

void deserializeVSGrid(const json& j, std::unordered_map<INT, std::unordered_map<INT, BOOL>> grid[2]) {
    // �����������
    grid[0].clear();
    grid[1].clear();

    // �����л���һ������
    if (j.contains("grid0")) {
        for (const auto& [yStr, row] : j["grid0"].items()) {
            INT y = std::stoi(yStr);
            for (const auto& [xStr, value] : row.items()) {
                INT x = std::stoi(xStr);
                grid[0][y][x] = value.get<BOOL>();
            }
        }
    }

    // �����л��ڶ�������
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

// ��ʾ�ļ�ѡ��Ի���
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
    ofn.lpstrTitle = L"ѡ�������ļ�";
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileName(&ofn) == TRUE) {
        return std::wstring(szFile);
    }

    return L"";
}

// �����غ���
std::unordered_map<INT, std::unordered_map<INT, BOOL>> loadGrid(HWND hWnd) {
    // ��ʾ�ļ�ѡ��Ի���
    std::wstring filepath = openFileDialog(hWnd);
    if (filepath.empty()) {
        return std::unordered_map<INT, std::unordered_map<INT, BOOL>>();
    }

    try {
        // ��ȡ������JSON
        json gridJson = loadJsonFromFile(filepath);
        std::unordered_map<INT, std::unordered_map<INT, BOOL>> grid = deserializeGrid(gridJson);
        return grid;
    }
    catch (const std::exception& e) {
        MessageBoxA(hWnd, e.what(), "���ش���", MB_ICONERROR);
        return std::unordered_map<INT, std::unordered_map<INT, BOOL>>(); // ���ؿ�����
    }
}

void loadVSGrid(HWND hWnd, std::unordered_map<INT, std::unordered_map<INT, BOOL>> grid[2]) {
    // ��ʾ�ļ�ѡ��Ի���
    std::wstring filepath = openFileDialog(hWnd);
    if (filepath.empty()) {
        MessageBox(hWnd, L"�û�ȡ������", L"NULL", MB_ICONERROR);
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
        MessageBox(hWnd, L"�����ļ�ʧ��", L"����", MB_ICONERROR);
        return;
    }
}