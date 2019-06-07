LINUX_CC=gcc
WINDOWS_CC=x86_64-w64-mingw32-gcc
CFLAGS= -g -Wall -static
LIBS= libmp3lame.a -lm -pthread

DEBUG =  -I"./lame/include" -pthread
TEST = gcc -o main.out main.c

main.out: main.o
	$(LINUX_CC) $(CFLAGS) -o main.out main.o $(LIBS)
main.o: main.c main.h
	$(LINUX_CC) $(CFLAGS) -c main.c -o main.o
clean:
	rm -f *.o main.out
clean_mp3:
	# deletes all mp3 files in the folder, not just recently encoded ones
	# TODO check for file date etc
	rm -f ./wavFiles/*.mp3 
run: main.out
	./main.out ./wavFiles/
test:
	./main.out ./wavFiles/
	./main.out ./wavFiles
	#add more tests, empty folder etc







lame_linux:
	$(LINUX_CC) $(CFLAGS)  main.c
	$(WINDOWS_CC) $(CFLAGS) main.o -o main.out

lame_windows:
	$(WINDOWS_CC) $(CFLAGS) -o main.out main.c







all:


