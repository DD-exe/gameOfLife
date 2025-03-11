/*
	用于实现生命游戏规则下数据的迭代
*/
#include "framework.h"

void myLife(std::unordered_map<INT, std::unordered_map<INT, BOOL>>& grid,
	std::unordered_map<INT, std::unordered_map<INT, BOOL>>& ans,
	INT xux,INT xuy,INT rex,INT rey,STATE** state) {
	unsigned int bitmap = 0; int x, y;
	INT stateCode = xux * 9 * 9 * 9 + xuy * 9 * 9 + rex * 9 + rey;
	if (state[stateCode] == nullptr) {
		state[stateCode] = new STATE[1 << 9];
		for (bitmap = 0; bitmap < 1 << 9; bitmap++) {
			for (x = y = 0; y < 9; y++)
				if (bitmap & 1 << y)					// 读第y个是否为1
					x += 1;								// 计数
			if (bitmap & 020) {
				if (x >= xux+1 && x <= xuy+1)
					state[stateCode][bitmap] = LIVE;  
				else
					state[stateCode][bitmap] = DEAD;
			}
			else {
				if (x >= rex && x <= rey)
					state[stateCode][bitmap] = LIVE;
				else
					state[stateCode][bitmap] = DEAD;
			}
		}
	}
	INT prev = 0; INT that = 1; INT next = 2;
	INT size = (INT)grid.size();
	bitmap = 0; // debug1
	for (INT done = 0; done < size + 3; done += grid.count(prev-1)) {
		INT jump = grid.count(prev) + grid.count(that) + grid.count(next);
		if (jump > 0) {
			INT max = getAll(grid, prev, that, next);
			INT xdone = 0;
			for (INT x = 0; xdone < max + 3; x++) {
				if (findLife(grid, x, prev)) {
					bitmap |= 0100;
					++xdone;
				}
				if (findLife(grid, x, that)) {
					bitmap |= 0200;
					++xdone;
				}
				if (findLife(grid, x, next)) {
					bitmap |= 0400;
					++xdone;
				}
				if (x - 1 >= 0) {
					if (state[stateCode][bitmap] == LIVE) {
						ans[that][x - 1] = TRUE;
					}
				}
				bitmap >>= 3;
				if (xdone >= max)xdone++;
			}
		}
		++prev; ++that; ++next;
		if (done >= size) { 
			done++; 
		}
	}
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