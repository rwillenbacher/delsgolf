
gcc compile.c export.c main.c `pkg-config gtk+-2.0 --cflags --libs` -o editor.exe