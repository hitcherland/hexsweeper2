#ifndef __sweeper_cell__
#define __sweeper_cell__

#include <stdio.h>
#include <stdlib.h>

class Cell {
	public:
		int id;
		int isMine;
		int mineCount;
		int activated;
		Cell* neighbours[ 6 ];
        //virtual void activate();
        //virtual void focus();
        //virtual void unfocus();
};

#endif
