CC := gcc
CFLAGS := -Wall

all:
	$(CC) $(CFLAGS) main.c -lbz2 -o bzthread

run:
	./bzthread  $(o) $(d)

check:
	cd $(o); find . -type f -exec md5sum "{}" \; > /tmp/checksum
	tar xf $(d).bz2.tar; cd $(d).bz2; find . -type f -exec bunzip2 "{}" \;; md5sum -c /tmp/checksum
