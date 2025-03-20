/*
	用于实现生命游戏规则下数据的迭代
*/
#include "framework.h"

void myLife(std::unordered_map<INT, std::unordered_map<INT, BOOL>>& grid,
	std::unordered_map<INT, std::unordered_map<INT, BOOL>>& ans,
	INT4& rule,STATE** state) {
	unsigned int bitmap = 0; int x, y;
	INT stateCode = rule.x * 9 * 9 * 9 + rule.y * 9 * 9 + rule.z * 9 + rule.t;
	if (state[stateCode] == nullptr) {
		state[stateCode] = new STATE[1 << 9];
		for (bitmap = 0; bitmap < 1 << 9; bitmap++) {
			for (x = y = 0; y < 9; y++)
				if (bitmap & 1 << y)					// 读第y个是否为1
					x += 1;								// 计数
			if (bitmap & 020) {
				if (x >= rule.x+1 && x <= rule.y+1)
					state[stateCode][bitmap] = LIVE;  
				else
					state[stateCode][bitmap] = DEAD;
			}
			else {
				if (x >= rule.z && x <= rule.t)
					state[stateCode][bitmap] = LIVE;
				else
					state[stateCode][bitmap] = DEAD;
			}
		}
	}
	INT prev = 0; INT that = 1; INT next = 2;
	INT size = (INT)grid.size();
	bitmap = 0; // debug1
	// HRGN xy = CreateRectRgn(0, 0, 0, 0);
	// INT maxx = 0;
	for (INT done = 0; done < size + 3; done += grid.count(prev-1)) {
		INT jump = grid.count(prev) + grid.count(that) + grid.count(next);
		if (jump > 0) {
			INT max = getAll(grid, prev, that, next);
			INT xdone = 0; INT x = 0; 
			BOOL old1 = FALSE; BOOL old2 = FALSE;
			for (x = 0; xdone < max + 3; x++) {
				if (findLife(grid, x, prev)) {
					// addXY(rectXY, xy, x, prev, cellSize);
					bitmap |= 0100;
					++xdone;
				}
				if (old1=findLife(grid, x, that)) {
					bitmap |= 0200;
					++xdone;
				}
				if (findLife(grid, x, next)) {
					bitmap |= 0400;
					++xdone;
				}
				if (x - 1 >= 0) {
					if (state[stateCode][bitmap] == LIVE) {
						if (old2)ans[that][x - 1] = FALSE;
						else ans[that][x - 1] = TRUE;		// 区分进攻和防御
					}
				}
				bitmap >>= 3;
				if (xdone >= max)xdone++;
				old2 = old1;
			}
			// if (x > maxx)maxx = x;
		}
		++prev; ++that; ++next;
		if (done >= size) { 
			done++; 
		}
	}
	// DeleteObject(xy);
}

BOOL findLife(std::unordered_map<INT, std::unordered_map<INT, BOOL>> &grid,INT x, INT y) {
	return grid.count(y) && grid[y].count(x);
}

void exchangeLife(std::unordered_map<INT, std::unordered_map<INT, BOOL>>& grid, INT x, INT y) {
	if (findLife(grid, x, y)) { 
		grid[y].erase(x);
		if (grid[y].empty()) {
			grid.erase(y);
		}
	}
	else grid[y][x] = TRUE;
}
/*
void addXY(HRGN origin,HRGN tmp,INT x,INT y,INT cellSize) {
	SetRectRgn(tmp, x * cellSize, (x + 1) * cellSize, y * cellSize, (y + 1) * cellSize);
	CombineRgn(origin, origin, tmp, RGN_OR);
}
*/