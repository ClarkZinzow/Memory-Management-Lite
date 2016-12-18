all:
	gcc -c -fpic mem1.c -Wall -Werror
	gcc -c -fpic mem2.c -Wall -Werror
	gcc -c -fpic mem3.c -Wall -Werror
	gcc -shared -o libmem1.so mem1.o
	gcc -shared -o libmem2.so mem2.o
	gcc -shared -o libmem3.so mem3.o

clean:
	rm -rf *o
