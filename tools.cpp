#include "framework.h"
#include "gameOfLife.h"

void getClientXY(HWND hWnd,INT* x,INT* y) {
    RECT clientRect;
    GetClientRect(hWnd, &clientRect);  // ��ȡ�ͻ�������
    *x = clientRect.right - clientRect.left;   // �ͻ������
    *y = clientRect.bottom - clientRect.top;  // �ͻ����߶�
}