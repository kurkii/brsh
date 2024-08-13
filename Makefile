BASE = src/main.c src/parser.c src/error.c src/pipe.c src/builtin.c
BUILD = target
CFLAGS = -Wall -Wextra -Wpedantic
all:
	mkdir target || true
	gcc $(BASE) $(CFLAGS) -o $(BUILD)/brsh

debug:
	gcc $(BASE) $(CFLAGS) -ggdb3 -o $(BUILD)/brsh	

clean:
	rm target/*
