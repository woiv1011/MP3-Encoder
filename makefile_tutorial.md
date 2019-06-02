###


output: main.o message.o //object files
	g++ main.o message.o -o output //compile both files into single executable

main.o: main.cpp
	g++ -c main.cpp //-c = dont create executable, just object file

message.o: message.cpp message.h //message.h necessary
	g++ -c message.cpp


target: dependencies
	action //exactly one tab

makefile looks at top target as primary target



makefile doesnt resolve includes by itself, only the compiler does, you have to include header files in dependencies


touch main.cpp - doesnt change the file, but gives it a new timestamp, for make 