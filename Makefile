CFLAGS := -g -O0

all: server.elf client.elf

server.elf: server.c protocol.h protocol.c
	gcc $(CFLAGS) -o server.elf server.c protocol.c

client.elf: client.c protocol.h protocol.c
	gcc $(CFLAGS) -o client.elf client.c protocol.c
