#pragma once
#pragma comment(lib, "gdiplus.lib")
#define _AFXDLL // 莫名其妙
#include "resource.h"
// GDIplus所需MFC
#include <afxdtctl.h>
#include <gdiplus.h>

void myPaintFrame(Gdiplus::Graphics& graphics, int x, int y, int w, int h, int cellSize); // tools