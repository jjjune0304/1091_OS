#include "syscall.h"

main()
        {
        	int     n;
        	for (n=1;n<=3;n++) {
		PrintInt(300+n);
		Sleep(1);
	}
        }