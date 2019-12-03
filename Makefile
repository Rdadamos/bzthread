CC := gcc
CFLAGS := -Wall

all:
	$(CC) $(CFLAGS) main.c -lbz2 -o bzthread

run:
	./bzthread  $(origin) $(destiny)
