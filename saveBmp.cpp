/*
    用于实现内置的模拟结果保存功能
*/
#include "framework.h"

void saveBmp(HWND hWnd, INT x, INT y, INT dx, INT dy) {
    HDC hdcWindow = GetDC(hWnd);
    HDC hdcMem = CreateCompatibleDC(hdcWindow);
    HBITMAP hBitmap = CreateCompatibleBitmap(hdcWindow, dx, dy);
    SelectObject(hdcMem, hBitmap);
   
    BitBlt(hdcMem, 0, 0, dx, dy, hdcWindow, x, y, SRCCOPY);// 从窗口中复制指定区域
    // 获取文件保存路径
    OPENFILENAME ofn;
    WCHAR szFile[260]; szFile[0] = L'\0';
    if (ofnRead(hWnd,ofn,szFile,260) == FALSE) { // 取消        
        DeleteObject(hBitmap);
        DeleteDC(hdcMem);
        ReleaseDC(hWnd, hdcWindow);
        return;
    }

    FILE* file;
    errno_t err=_wfopen_s(&file,szFile, L"wb");
    if (!file||err!=NULL) {
        MessageBox(hWnd, L"保存文件出错", L"Error", MB_OK);
        DeleteObject(hBitmap);
        DeleteDC(hdcMem);
        ReleaseDC(hWnd, hdcWindow);
        return;
    }
    
    BITMAPFILEHEADER bitFileHeader;
    BITMAPINFOHEADER bitInfoHeader;
    INT duiqiX = (dx * 3 + 3) & ~3; // 堆错误解决方案:内存对齐问题
    bfhWrite(bitFileHeader, file, dx, dy, duiqiX);
    bihWrite(bitInfoHeader, file, dx, dy, duiqiX);
    BYTE* pixels = new BYTE[dy * duiqiX]; 
    if (pixels != nullptr) {
        GetDIBits(hdcMem, hBitmap, 0, dy, pixels, (BITMAPINFO*)&bitInfoHeader, DIB_RGB_COLORS);
        fwrite(pixels, bitInfoHeader.biSizeImage, 1, file);
        delete[]pixels;
    }
    fclose(file);
    DeleteObject(hBitmap);
    DeleteDC(hdcMem);
    ReleaseDC(hWnd, hdcWindow);
}

BOOL ofnRead(HWND hWnd,OPENFILENAME& ofn, WCHAR* szFile,DWORD bufSize) {
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hWnd;
    ofn.lpstrFile = szFile;
    ofn.lpstrFile[0] = L'\0';
    ofn.nMaxFile = bufSize;
    ofn.lpstrFilter = L"BMP Files\0*.BMP\0All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.lpstrTitle = L"保存记录";
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_OVERWRITEPROMPT;

    return GetSaveFileName(&ofn);
}

void bfhWrite(BITMAPFILEHEADER& bfh, FILE* file,INT dx,INT dy,INT duiqiX) {
    bfh.bfType = 0x4D42; // bmp
    bfh.bfReserved1 = 0;
    bfh.bfReserved2 = 0;
    bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    bfh.bfSize = bfh.bfOffBits + (dy * duiqiX);  // 每个像素占用3字节
    fwrite(&bfh, sizeof(BITMAPFILEHEADER), 1, file);
}

void bihWrite(BITMAPINFOHEADER& bih, FILE* file, INT dx, INT dy, INT duiqiX) {
    bih.biSize = sizeof(BITMAPINFOHEADER);
    bih.biWidth = dx;
    bih.biHeight = dy;
    bih.biPlanes = 1;
    bih.biBitCount = 24;
    bih.biCompression = 0;
    bih.biSizeImage = dy * duiqiX;
    bih.biXPelsPerMeter = 0;
    bih.biYPelsPerMeter = 0;
    bih.biClrUsed = 0;
    bih.biClrImportant = 0;
    fwrite(&bih, sizeof(BITMAPINFOHEADER), 1, file);
}