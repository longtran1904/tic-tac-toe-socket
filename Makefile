CC = gcc
CFLAGS = -std=c99 -g -Wall -fsanitize=address,undefined

client: ttt.c protocol.c
	$(CC) $(CFLAGS) $^ -DDEBUG=0 -o client
server: ttts.c
	$(CC) $(CFLAGS) $< -DDEBUG=0 -o server
protocol: test_protocol.c protocol.c
	$(CC) $(CFLAGS) $^ -DDEBUG=0 -o test
clean: 
	rm -f -r server client test *.dSYM