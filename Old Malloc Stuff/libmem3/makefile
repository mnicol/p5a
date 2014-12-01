libmem1:
	gcc -c -fpic mem.c -Wall -Werror
	gcc -shared -o libmem1.so mem.o

libmem2:
	gcc -c -fpic mem.c -Wall -Werror
	gcc -shared -o libmem2.so mem.o

libmem3:
	gcc -c -fpic mem.c -Wall -Werror
	gcc -shared -o libmem3.so mem.o

mymain:
	gcc -lmem1 -L. -o myprogram mymain.c -Wall -Werror
