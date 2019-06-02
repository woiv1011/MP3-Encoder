@echo off
cls
REM tcc main.c -g -I"C:\Users\Ivar\Desktop\MP3-Encoder\lame-3.100\include" -vv
REM -vv to show included files
REM -L"C:\Users\Ivar\Desktop\MP3-Encoder\lame-3.100\include"
REM where command = which
REM main "C:\Users\Ivar\Desktop\MP3-Encoder"
REM main "../wavFiles/"
main.exe "./wavFiles/"