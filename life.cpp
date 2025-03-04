#include "framework.h"

void myLife(std::unordered_map<INT, std::unordered_map<INT, BOOL>>& grid,
	std::unordered_map<INT, std::unordered_map<INT, BOOL>>& ans) {
	unsigned int bitmap = 0; int x, y;
	static enum { DEAD, LIVE } state[1 << 9];
	if (state[007] == DEAD) {							//读活例子，避免重复初始化
		for (bitmap = 0; bitmap < 1 << 9; bitmap++) {
			for (x = y = 0; y < 9; y++)
				if (bitmap & 1 << y)					// 读第y个是否为1
					x += 1;								// 计数
			if (bitmap & 020) {
				if (x == 2 || x == 3)
					state[bitmap] = LIVE;  
				else
					state[bitmap] = DEAD;
			}
			else {
				if (x == 3)
					state[bitmap] = LIVE;
				else
					state[bitmap] = DEAD;
			}
		}
	}
	INT prev = 0; INT that = 1; INT next = 2;
	INT size = (INT)grid.size();
	for (INT done = 0; done < size; done += grid.count(prev)) {
		INT jump = grid.count(prev) + grid.count(that) + grid.count(next);
		if (jump > 0) {
			INT max = (INT)grid[prev].size() + (INT)grid[that].size() + (INT)grid[next].size();
			INT xdone = 0;
			for (INT x = 0; xdone < max;x++) {
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
					if (state[bitmap] == LIVE) {
						ans[that][x - 1] = TRUE;
					}
				}
				bitmap >>= 3;
			}
		}
		++prev; ++that; ++next;
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

void lifeOrigin(int *that, int *newi) // 此函数学习于https://dotat.at/prog/life/life.html
								// List Life algorithm(by Tony Finch)
{
	unsigned int bitmap; // notice
	int *next, *prev;
	int x, y;
	static enum{DEAD,LIVE} state[1 << 9];
	if(state[007] == 0) {							//读活例子，避免重复初始化
		for(bitmap = 0; bitmap < 1<<9; bitmap++) {
			for(x = y = 0; y < 9; y++)
				if(bitmap & 1<<y)					// 读第y个是否为1
					x += 1;							//计数
			if(bitmap & 020) {
				if(x == 2 || x == 3)
					state[bitmap] = LIVE;
				else
					state[bitmap] = DEAD;
			} else {
				if(x == 3)
					state[bitmap] = LIVE;
				else
					state[bitmap] = DEAD;
			}
		}
	} // 初始化9位状态->LIVE/DEAD的映射

	prev = next = that;
	bitmap = 0;
	*newi = 0;
	for(;;) {
		/* did we write an X co-ordinate? */
		if(*newi < 0)
			newi++;
		if(prev == next) {
			/* start a new group of rows */
			if(*next == 0) {
				*newi = 0;
				return;
			}
			y = *next + 1; next++;
		}
		else {
			/* move to next row and work out which ones to scan */
			y--;
			if (*prev == y + 1)
				prev++;
			if (*that == y)
				that++;
			if (*next == y - 1)
				next++;
		}
		/* write new row co-ordinate */
		*newi = y;
		for(;;) {
			/* skip to the leftmost cell */
			x = *prev;
			if(x > *that)
				x = *that;
			if(x > *next)
				x = *next;
			/* end of line? */
			if(x >= 0)
				break;
			for(;;) {
				/* add a column to the bitmap */
				if(*prev == x) {
					bitmap |= 0100;
					prev++;
				}
				if(*that == x) {
					bitmap |= 0200;
					that++;
				}
				if(*next == x) {
					bitmap |= 0400;
					next++;
				}
				/* what does this bitmap indicate? */
				if(state[bitmap] == LIVE)
					*++newi = x - 1;
				else if(bitmap == 000)
					break;
				/* move right */
				bitmap >>= 3;
				x += 1;
			}
		}
	}
}