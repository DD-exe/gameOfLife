#include "framework.h"
#include "gameOfLife.h"

void myPaintFrame(Gdiplus::Graphics& graphics, int x, int y, int w, int h, int cellSize)
{
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