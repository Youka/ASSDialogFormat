all:
	gcc -O2 -Os -c main.c -o main.o
	g++ -o ass_dialog_format main.o -s
clean:
	rm -f main.o ass_dialog_format