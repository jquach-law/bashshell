main:
	gcc -std=gnu99 -Wall -g -o smallsh smallsh.c

run:
	smallsh

clean:
	rm -f smallsh