prom = myprogram

$(prom): mymain.c mem.c mem.h
	$ gcc -c -fpic mem.c -Wall -Werror
	$ gcc -shared -o libmem.so mem.o
	$ export LD_LIBRARY_PATH=$ LD_LIBRARY_PATH:.
	$ gcc -lmem -L. -o myprogram mymain.c -Wall -Werror
