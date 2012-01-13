CC := gcc
override __CFLAGS := -O2 -g -std=gnu99 $(CFLAGS)

LD := $(CC)
override __LDFLAGS := -lgmp $(LDFLAGS)

.PHONY: all
all: pipi

pipi: src/main.o
	$(LD) $(__LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(__CFLAGS) -c -o $@ $^

.PHONY: clean
clean:
	find -name '*.o' -delete
	rm -f pipi
