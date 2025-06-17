CC := clang
CFLAGS := -std=c99
CFLAGS += -Wall -Wextra -Wpedantic
CFLAGS += -lmarkdown

all: CFLAGS += -O3
all: clean msg

debug: CFLAGS += -O0 -g3 -glldb
debug: clean msg

msg: msg.c
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f msg
