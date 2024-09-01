CC := gcc
CFLAGS := -g -Wall -Wno-unused-variable -fsanitize=address,undefined

all: myShell

myShell: myShell.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f myShell