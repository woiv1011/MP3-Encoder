LINUX_CC=gcc
WINDOWS_CC=x86_64-w64-mingw32-gcc
CFLAGS=-g -I"./lame-3.100/include" -Wall -static

main.out: main.o
	$(LINUX_CC) $(CFLAGS) main.o -o main.out
	
main.o: main.c main.h
	$(LINUX_CC) $(CFLAGS) -c main.c -o main.o
clean:
	rm -f *.o main.out 
run: main.out
	./main.out ./wavFiles/






linux: main.o
	$(LINUX_CC) $(CFLAGS)  main.c
	$(WINDOWS_CC) $(CFLAGS) main.o -o main.out

windows: main.o
	$(WINDOWS_CC) $(CFLAGS) -o main.out main.c







all:


