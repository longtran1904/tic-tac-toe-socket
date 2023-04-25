CC = gcc
CFLAGS = -std=c99 -g -Wall -fsanitize=address,undefined -pthread

client: ttt.c protocol.c
	$(CC) $(CFLAGS) $^ -DDEBUG=0 -o client
server: ttts.c protocol.c game_database.c game_logic.c socket_buf_mem.c
	$(CC) $(CFLAGS) $^ -DDEBUG=0 -o server
protocol_tester: test_protocol.c protocol.c
	$(CC) $(CFLAGS) $^ -DDEBUG=0 -o test_protocol
server_tester: test_server.c
	$(CC) $(CFLAGS) $^ -DDEBUG=0 -o test_server
clean: 
	rm -f -r server client test_protocol test_server *.dSYM
