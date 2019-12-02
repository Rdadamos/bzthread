CC := gcc
CFLAGS := -Wall

all:
	$(CC) $(CFLAGS) main.c -o bzthread

run:
	./bzthread  $(origin) $(destiny)
