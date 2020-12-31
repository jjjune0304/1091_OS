#include "syscall.h"
main()
	{
		int i;
    		int j;
    		for(i = 0; i < 3; i++){
        			for(j = 0; j < 5; j++){
            				PrintInt(100+j);
        			}
        			Sleep(1);
    		}

	}
