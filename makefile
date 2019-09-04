myprogram : mymain.c libmem.so
	gcc -o myprogram mymain.c -I. -L. -lmem -Wall -Werror

libmem.so : mem.c
	gcc -c -fpic mem.c -Wall -Werror
	gcc -shared -o libmem.so mem.o

clea :
	rm mem.o



