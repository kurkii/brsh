BASE = src/main.c src/parser.c src/error.c src/pipe.c src/builtin.c src/config.c
BUILD = target
CFLAGS = -Wall -Wno-unused-parameter -Wextra

all:
	mkdir target || true
	gcc $(BASE) $(CFLAGS) -o $(BUILD)/brsh

debug:
	gcc $(BASE) $(CFLAGS) -ggdb3 -o $(BUILD)/brsh	

clean:
	rm target/*
