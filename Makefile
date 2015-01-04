CC = gcc
CFLAGS = -Wall -Werror
RUN_ALL = run-01 run-02 run-03 run-04
COMPILE_ALL = test-01 test-02 test-03 test-04

.PHONY: all run run-% clean

all: $(COMPILE_ALL)

run: $(RUN_ALL)

run-%: test-%
	-./$<

test-%: test-%.o xmalloc.o
	$(CC) $(CFLAGS) $< xmalloc.o -o $@
test-%.o: test-%.c xmalloc.h
	$(CC) $(CFLAGS) -c $<
xmalloc.o: xmalloc.c xmalloc.h
	$(CC) $(CFLAGS) -c xmalloc.c


clean:
	-rm xmalloc.o $(COMPILE_ALL)
