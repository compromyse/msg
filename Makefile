BUILD := build
$(shell mkdir -p build)

SRC := $(wildcard src/*.c)
OBJ := $(SRC:src/%.c=$(BUILD)/%.o)
OUT := $(BUILD)/msg

FORMAT := clang-format
TIDY := clang-tidy

CC := clang
CFLAGS := -Wall -Wextra -Wpedantic -std=c99
CFLAGS += -Wno-unused-result
CFLAGS += -D_GNU_SOURCE -D_FORTIFY_SOURCE=2

all: release

$(OUT): $(OBJ)
	@printf "\e[33mLink\e[90m %s\e[0m\n" $@
	$(CC) $(CFLAGS) $^ -o $@
	@printf "\e[34mDone!\e[0m\n"

$(BUILD)/%.o: src/%.c | $(BUILD)
	@printf "\e[36mCompile\e[90m %s\e[0m\n" $@
	$(CC) $(CFLAGS) -c $< -o $@

release: CFLAGS += -Werror -O3 -s
release: clean $(OUT)

debug: CFLAGS += -O1 -g3 -glldb
debug: $(OUT)

format: $(wildcard src/*)
	@printf "\e[36mFormatting\e[0m\n"
	$(FORMAT) -i $^
	@printf "\e[33mTidying\e[0m\n"
	$(TIDY) $^
	@printf "\e[34mDone!\e[0m\n"

clean:
	@rm -f $(OUT) $(OBJ)
	@printf "\e[34mAll clear!\e[0m\n"
