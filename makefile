RISCV=/mnt/d/_ubuntu/_/riscv-glibc
# qemu-riscv64 -L $RISCV/sysroot ./rvemu
TOOLPREFIX=$(RISCV)/riscv64-unknown-linux-gnu-

# CC=$(TOOLPREFIX)gcc
# AS = $(TOOLPREFIX)gas
# LD = $(TOOLPREFIX)ld

CC=gcc
CFLAGS=-std=c11 -g -fno-common
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

minic: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(OBJS): minic.h

test: minic
	./test.sh

clean:
	rm -f minic *.o *~ tmp*

.PHONY: test clean
