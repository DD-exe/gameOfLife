/*
    ����ʵ�����õ�ģ�������湦��
*/
#include "framework.h"
#include "gameOfLife.h"
#include <windows.h>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <nlohmann/json.hpp>
#include <shlobj.h> // ���ڻ�ȡ�ĵ�Ŀ¼

using json = nlohmann::json;

// �� grid ת��Ϊ JSON
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

    // ���л���һ������ (grid[0])
    json grid0;
    for (const auto& [y, row] : grid[0]) {
        for (const auto& [x, value] : row) {
            grid0[std::to_string(y)][std::to_string(x)] = value;
        }
    }
    j["grid0"] = grid0;

    // ���л��ڶ������� (grid[1])
    json grid1;
    for (const auto& [y, row] : grid[1]) {
        for (const auto& [x, value] : row) {
            grid1[std::to_string(y)][std::to_string(x)] = value;
        }
    }
    j["grid1"] = grid1;

    return j;
}

// ����Ĭ��ʱ����ļ���
std::wstring generateDefaultFilename() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);

    std::wstringstream wss;
    tm tm_buf;
    localtime_s(&tm_buf, &in_time_t);
    wss << std::put_time(&tm_buf, L"grid_%Y%m%d_%H%M%S.json");

    return wss.str();
}

// ��ȡĬ�ϱ���Ŀ¼���ĵ�/GameOfLifeSaves/��
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

// ��ʾ����Ի��򣬴�Ĭ���ļ���
std::wstring showSaveDialog(HWND hWnd, const std::wstring& defaultFilename) {
    OPENFILENAME ofn = { 0 };
    WCHAR szFile[MAX_PATH] = { 0 };

    // ����Ĭ��·�����ļ���
    std::wstring defaultPath = getDefaultSaveDirectory() + defaultFilename;
    wcscpy_s(szFile, defaultPath.c_str());

    // ���öԻ���
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hWnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = L"JSON Files (*.json)\0*.json\0All Files (*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrDefExt = L"json";
    ofn.lpstrTitle = L"������������";
    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;

    if (GetSaveFileName(&ofn)) {
        return szFile;
    }

    return L""; // �û�ȡ��
}

// ����JSON��ָ��·��
bool saveJsonToFile(const json& j, const std::wstring& filepath) {
    try {
        std::ofstream outfile(filepath, std::ios::binary);
        if (!outfile.is_open()) {
            return false;
        }

        outfile << j.dump(4); // ���������4�ո�����
        return true;
    }
    catch (...) {
        return false;
    }
}

// single���溯��
bool saveGrid(HWND hWnd, const std::unordered_map<INT, std::unordered_map<INT, BOOL>>& grid) {
    // ���л�����
    json gridJson = serializeGrid(grid); // ʹ��֮ǰ�����serializeGrid

    // ׼��Ĭ���ļ���
    std::wstring defaultName = generateDefaultFilename();

    // ��ʾ����Ի���
    std::wstring savePath = showSaveDialog(hWnd, defaultName);
    if (savePath.empty()) {
        return false; // �û�ȡ��
    }

    // �����ļ�
    if (!saveJsonToFile(gridJson, savePath)) {
        MessageBox(hWnd, L"�����ļ�ʧ��", L"����", MB_ICONERROR);
        return false;
    }

    // ��ʾ�ɹ���Ϣ
    std::wstring successMsg = L"�����ѱ��浽:\n" + savePath;
    MessageBox(hWnd, successMsg.c_str(), L"����ɹ�", MB_ICONINFORMATION);

    return true;
}

// vs���溯��
bool saveVSGrid(HWND hWnd, const std::unordered_map<INT, std::unordered_map<INT, BOOL>> grid[2]) {
    // ���л�����
    json gridJson = serializeVSGrid(grid); // ʹ��֮ǰ�����serializeGrid

    // ׼��Ĭ���ļ���
    std::wstring defaultName = generateDefaultFilename();

    // ��ʾ����Ի���
    std::wstring savePath = showSaveDialog(hWnd, defaultName);
    if (savePath.empty()) {
        return false; // �û�ȡ��
    }

    // �����ļ�
    if (!saveJsonToFile(gridJson, savePath)) {
        MessageBox(hWnd, L"�����ļ�ʧ��", L"����", MB_ICONERROR);
        return false;
    }

    // ��ʾ�ɹ���Ϣ
    std::wstring successMsg = L"�����ѱ��浽:\n" + savePath;
    MessageBox(hWnd, successMsg.c_str(), L"����ɹ�", MB_ICONINFORMATION);

    return true;
}