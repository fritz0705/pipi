CC := gcc
override __CFLAGS := -Wall -Wextra -O2 -g -pthread -std=gnu99 $(CFLAGS)

LD := $(CC)
override __LDFLAGS := -lgmp -pthread $(LDFLAGS)

.PHONY: all
all: pipi

pipi: src/main.o src/bbp.o src/bbp-pi.o src/bellard.o
	$(LD) $(__LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(__CFLAGS) -c -o $@ $^

.PHONY: clean
clean:
	find -name '*.o' -delete
	rm -f pipi
