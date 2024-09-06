BUILD := build
$(shell mkdir -p build)

SRC := $(wildcard src/*.c)
OBJ := $(SRC:src/%.c=$(BUILD)/%.o)
OUT := $(BUILD)/msg

CC := gcc
CFLAGS := -Wall -Wextra -Wpedantic

$(OUT): $(OBJ)
	@printf "\e[33mLinking\e[90m %s\e[0m\n" $@
	$(CC) $(CFLAGS) $^ -o $@
	@printf "\e[34mDone!\e[0m\n"

$(BUILD)/%.o: src/%.c | $(BUILD)
	@printf "\e[36mCompile\e[90m %s\e[0m\n" $@
	$(CC) $(CFLAGS) -c $< -o $@

release: CFLAGS += -Werror -O3
release: clean $(OUT)

clean:
	@rm -f $(OUT) $(OBJ)
	@printf "\e[34mAll clear!\e[0m\n"
