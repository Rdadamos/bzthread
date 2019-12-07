CC := gcc
CFLAGS := -Wall

all:
	$(CC) $(CFLAGS) main.c -lm -O2 -static -lpthread -lbz2 -o bzthread

run:
	./bzthread  $(o) $(d)
	echo $(d) >> remove_list

check:
	cd $(o); find . -type f -exec md5sum "{}" \; > /tmp/checksum
	tar xf $(d).bz2.tar; echo $(d) >> remove_list; cd $(d); find . -type f -exec bunzip2 "{}" \;; md5sum -c /tmp/checksum

clean:
	xargs rm -r < remove_list
	rm remove_list
