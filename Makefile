CC      = clang
CFLAGS  = -Wall -Wextra -O2 -MMD -MP -Iinclude
LDFLAGS =

SRC_DIR   = src
INC_DIR   = include
BUILD_DIR = build
BIN_DIR   = bin

# entry points (each makes a program)
PROGS = main test

# discover all .c files under src
SRCS  := $(wildcard $(SRC_DIR)/*.c)

# object files live in build/, mirroring src/ names
OBJS  := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))

# per-program object lists: link each entry point with the common modules
COMMON_OBJS := $(filter-out $(BUILD_DIR)/main.o $(BUILD_DIR)/test.o, $(OBJS))

all: $(addprefix $(BIN_DIR)/,$(PROGS))

# Link rules
$(BIN_DIR)/main: $(BUILD_DIR)/main.o $(COMMON_OBJS) | $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

$(BIN_DIR)/test: $(BUILD_DIR)/test.o $(COMMON_OBJS) | $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# Compile rule: .c -> build/.o (+ emits build/.d via -MMD -MP)
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Ensure output dirs exist
$(BUILD_DIR) $(BIN_DIR):
	mkdir -p $@

# Housekeeping
.PHONY: all clean distclean
clean:
	rm -f $(BUILD_DIR)/*.o $(BUILD_DIR)/*.d

distclean: clean
	rm -f $(BIN_DIR)/*

# Include auto-generated header dependencies
-include $(OBJS:.o=.d)