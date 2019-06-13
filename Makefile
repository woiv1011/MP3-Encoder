CC=gcc
CFLAGS= -g -Wall -static
LIBS= libmp3lame_linux.a -lm -pthread

WINDOWS_CC=x86_64-w64-mingw32-gcc
WINDOWS_CFLAGS= -g -Wall -static -DPTW32_STATIC_LIB
WINDOWS_LIBS=libmp3lame_windows.a -lm -pthread

main.out: main.o
	$(CC) $(CFLAGS) -o main.out main.o $(LIBS)
main.o: main.c main.h
	$(CC) $(CFLAGS) -c main.c -o main.o
clean:
	rm -f *.o main.out main.exe
clean_mp3:
	# deletes all mp3 files in the folder, not just recently encoded ones
	# TODO check for file date etc to prevent deleting old mp3 files
	rm -f ./wavFiles/*.mp3 
run: main.out
	./main.out ./wavFiles/

windows: windows_main.o
	$(WINDOWS_CC) $(WINDOWS_CFLAGS) -o main.exe windows_main.o $(WINDOWS_LIBS)
windows_main.o: main.c main.h
	$(WINDOWS_CC) $(WINDOWS_CFLAGS) -c main.c -o windows_main.o

