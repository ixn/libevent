
CC=gcc
CFLAGS=-I.

all: server_kalkulator
server_kalkulator: server_kalkulator.c
	$(CC) $(CFLAGS) -o server_kalkulator $^ -L/usr/local/lib -levent -lm $(LDFLAGS)
	
clean:
	rm -f *.o
	rm -f *~
	rm -f server_kalkulator
