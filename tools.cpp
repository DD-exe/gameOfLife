/*
    用于实现其他部分用到的小工具
*/

#include "framework.h"
#include "gameOfLife.h"
#include <random>

void myPaintFrame(Gdiplus::Graphics& graphics, int x, int y, int w, int h, int cellSize)
{
    if (cellSize <= 0)return;
    Gdiplus::Pen pen(Gdiplus::Color(255, 200, 200, 200), 0.1f);
    // 绘制垂直线
    for (int i = 0; i <= w; i += cellSize) {
        graphics.DrawLine(&pen, x + i, y, x + i, y + h);
    }
    // 绘制水平线
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
    std::random_device rd;  // 随机种子
    std::mt19937 gen(rd()); // Mersenne Twister 19937 生成器
    std::uniform_int_distribution<int> distrib(min, max);
    return distrib(gen);
}