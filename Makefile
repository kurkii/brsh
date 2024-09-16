BASE = src/main.c src/parser.c src/error.c src/pipe.c src/builtin.c src/config.c
BUILD = target
CC = gcc
CFLAGS = -Wall -Wno-unused-parameter -Wextra
CDEBUG = -ggdb3 -DDEBUG

all:
	mkdir $(BUILD) || true
	$(CC) $(BASE) $(CFLAGS) -o $(BUILD)/brsh

debug:
	$(CC) $(BASE) $(CFLAGS) $(CDEBUG) -o $(BUILD)/brsh	

clean:
	rm target/*
