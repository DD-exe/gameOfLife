/*
    ����ʵ�����������õ���С����
*/

#include "framework.h"
#include "gameOfLife.h"
#include <random>

void myPaintFrame(Gdiplus::Graphics& graphics, int x, int y, int w, int h, int cellSize)
{
    if (cellSize <= 0)return;
    Gdiplus::Pen pen(Gdiplus::Color(255, 200, 200, 200), 0.1f);
    // ���ƴ�ֱ��
    for (int i = 0; i <= w; i += cellSize) {
        graphics.DrawLine(&pen, x + i, y, x + i, y + h);
    }
    // ����ˮƽ��
    for (int i = 0; i <= h; i += cellSize) {
        graphics.DrawLine(&pen, x, y + i, x + w, y + i);
    }
}

INT getAll(std::unordered_map<INT, std::unordered_map<INT, BOOL>>& grid,INT x, INT y, INT z) {
    INT ans = 0;
    if (grid.count(x)) {
        ans += (INT)grid[x].size();
    }
    if (grid.count(y)) {
        ans += (INT)grid[y].size();
    }
    if (grid.count(z)) {
        ans += (INT)grid[z].size();
    }

    return ans;
}

void delState(STATE** x) {
    for (int i = 0; i < 9 * 9 * 9 * 9; ++i) {
        if (x[i] != nullptr) {
            delete[]x[i];
        }
    }
    delete[]x;
}

int getRandomNum(int min, int max) {
    std::random_device rd;  // �������
    std::mt19937 gen(rd()); // Mersenne Twister 19937 ������
    std::uniform_int_distribution<int> distrib(min, max);
    return distrib(gen);
}

std::string wc2s(const wchar_t* wstr) {
    int len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, nullptr, 0, nullptr, nullptr);
    std::string str(len, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr, -1, &str[0], len, nullptr, nullptr);
    return str;
}