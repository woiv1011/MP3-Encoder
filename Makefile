LINUX_CC=gcc
WINDOWS_CC=x86_64-w64-mingw32-gcc
CFLAGS= -g -Wall -static
LIBS= libmp3lame.a -lm

DEBUG =  -I"./lame/include"
TEST = gcc -o main.out main.c

main.out: main.o
	$(LINUX_CC) $(CFLAGS) -o main.out main.o $(LIBS)
main.o: main.c main.h
	$(LINUX_CC) $(CFLAGS) -c main.c -o main.o
clean:
	rm -f *.o main.out 
run: main.out
	./main.out ./wavFiles/






lame_linux:
	$(LINUX_CC) $(CFLAGS)  main.c
	$(WINDOWS_CC) $(CFLAGS) main.o -o main.out

lame_windows:
	$(WINDOWS_CC) $(CFLAGS) -o main.out main.c







all:


