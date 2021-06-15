CC=gcc
# CC=gcc -Wall
mysh: get_path.o which.o where.o shell-with-builtin.o add.o print.o delete.o freeList.o
	$(CC) -g -pthread shell-with-builtin.c get_path.o where.o which.o add.o print.o delete.o freeList.o -o mysh

shell-with-builtin.o: shell-with-builtin.c sh.h
	$(CC) -g -c shell-with-builtin.c

get_path.o: get_path.c get_path.h
	$(CC) -g -c get_path.c

which.o: which.c get_path.h
	$(CC) -g -c which.c

where.o: where.c get_path.h
	$(CC) -g -c where.c

add.o: add.c user.h
	gcc -g -c add.c

print.o: print.c user.h
	gcc -g -c print.c

freeList.o: freeList.c user.h
	gcc -g -c freeList.c

delete.o: delete.c user.h
	gcc -g -c delete.c

test: test-1+2.c
	gcc -g -o test test-1+2.c

clean:
	rm -rf *.o mysh test
