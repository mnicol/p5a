mfs:
	gcc -c -fpic mfs.c -Wall -Werror
	gcc -shared -o libmfs.so mfs.o

server:
	gcc -lmfs -L. -o server server.c -Wall -Werror

clean: 
	rm *.o -rf
	rm *.so -rf