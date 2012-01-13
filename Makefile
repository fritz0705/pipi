CC := gcc
override __CFLAGS := -Wall -Wextra -O2 -g -pthread -std=gnu99 $(CFLAGS)

LD := $(CC)
override __LDFLAGS := -lgmp -pthread $(LDFLAGS)

THREADS :=$(shell grep "^processor" /proc/cpuinfo | wc -l)

.PHONY: all
all: pipi

.PHONY: threaded
threaded: __CFLAGS += -D WITH_THREADING -D THREADS=$(THREADS)
threaded: pipi

pipi: src/main.o src/bbp.o
	$(LD) $(__LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(__CFLAGS) -c -o $@ $^

.PHONY: clean
clean:
	find -name '*.o' -delete
	rm -f pipi
