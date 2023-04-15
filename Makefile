CC = gcc
CFLAGS = -std=c99 -g -Wall -fsanitize=address,undefined

client: ttt.c
    $(CC) $(CFLAGS) $< -DDEBUG=1 -o client

server: ttts.c
	$(CC) $(CFLAGS) $< -DDEBUG=1 -o server

clean: 
	rm -f -r server client *.dSYM