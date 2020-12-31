#include "syscall.h"

main()
        {
        		int i;
    		int j;
    		for(i = 0; i < 3; i++){
        			for(j = 0; j < 10; j++){
            				PrintInt(200+j);
        			}
        			Sleep(1);
    		}
        }
