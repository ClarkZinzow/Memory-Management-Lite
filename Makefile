all:
	gcc -c -fpic -o build/mem1.o src/mem1.c -Wall -Werror
	gcc -c -fpic -o build/mem2.o src/mem2.c -Wall -Werror
	gcc -c -fpic -o build/mem3.o src/mem3.c -Wall -Werror
	gcc -shared -o lib/libmem1.so build/mem1.o
	gcc -shared -o lib/libmem2.so build/mem2.o
	gcc -shared -o lib/libmem3.so build/mem3.o

clean:
	rm -rf *o
