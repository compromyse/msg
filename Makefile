CC := clang
EXEC := msg
BUILD := build
BUILT := $(BUILD)/$(EXEC)

CMAKE_ARGS ?= -DCMAKE_EXPORT_COMPILE_COMMANDS=On -G 'Unix Makefiles' -DCMAKE_C_COMPILER=$(CC)
MAKE_ARGS ?= -j$(shell nproc)

all: $(BUILT)

.PHONY: $(BUILT)
$(BUILT): $(BUILD)
	cmake --build $(BUILD) -- $(MAKE_ARGS)

$(BUILD):
	cmake -S $(PWD) -B $@ $(CMAKE_ARGS)

clean:
	$(RM) -rf $(BUILD)
