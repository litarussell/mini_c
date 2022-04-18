CFLAGS=-std=c11 -g -fno-common

minic: main.o
	$(CC) -o minic main.o $(LDFLAGS)

test: minic
	./test.sh

clean:
	rm -f minic *.o *~ tmp*

.PHONY: test clean