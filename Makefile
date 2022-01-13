SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := bin
TEST_DIR := test

EXE := $(BIN_DIR)/life
SRC := $(wildcard $(SRC_DIR)/*.c)
OBJ := $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
TEST := $(wildcard $(TEST_DIR)/*.c)

CC       := gcc
CPPFLAGS := -Iinclude -MMD -MP
CFLAGS   := -g -Wall -Wextra
LDLIBS   := -lncurses

.PHONY: all clean tests

all: $(EXE)

$(EXE): $(OBJ) | $(BIN_DIR)
	$(CC) $^ $(LDLIBS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@

clean:
	@$(RM) -rv $(BIN_DIR) $(OBJ_DIR)

tests: $(OBJ) $(TEST)
	$(CC) $(CPPFLAGS) $(CFLAGS) test/test_life_rules.c $(OBJ) -o $(BIN_DIR)/test_life_rules

-include $(OBJ:.o=.d)
