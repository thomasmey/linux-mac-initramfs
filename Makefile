LDFLAGS=-static -m32
CFLAGS=-m32
CC=klcc
init: init.o
init.o: init.c
