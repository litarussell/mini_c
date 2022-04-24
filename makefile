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
