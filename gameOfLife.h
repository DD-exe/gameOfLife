#pragma once
#pragma comment(lib, "gdiplus.lib")
#define _AFXDLL // Ī������
#include "resource.h"
// GDIplus����MFC
#include <afxdtctl.h>
#include <gdiplus.h>

void myPaintFrame(Gdiplus::Graphics& graphics, int x, int y, int w, int h, int cellSize); // tools