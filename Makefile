src = $(wildcard *.c)
obj = $(src:.c=.o)
CC = gcc
CFLAGS = -Wall 
LDFLAGS =

server: $(obj)
	$(CC) $(CFLAFS) -o $@ $^ $(LDFLAGS)

.PHONY: clean
clean:
	rm -f $(obj) a.out
