#include "gameOfLife.h"


void life(int *that, int *newi) // 此函数学习于https://dotat.at/prog/life/life.html
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