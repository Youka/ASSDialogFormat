.PHONY: all clean

all:
	gcc -O2 -c main.c -o main.o
	gcc -o ass_dialog_format main.o -s

clean:
	rm -f main.o ass_dialog_format*