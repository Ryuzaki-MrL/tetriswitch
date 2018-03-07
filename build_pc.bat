@echo off
cd source
g++ *.cpp *.c -o ../test.exe -lsfml-graphics -lsfml-window -lsfml-system -static-libgcc
pause
