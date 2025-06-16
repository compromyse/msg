CC := clang
CFLAGS := -std=c99
CFLAGS += -Wall -Wextra -Wpedantic

all: CFLAGS += -O3
all: clean msg

debug: CFLAGS += -Og -g3 -glldb
debug: clean msg

msg: msg.c
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f msg
