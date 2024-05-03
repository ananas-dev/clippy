CC=cc
CFLAGS=-g -flto -O3
NAME=clippy

SRC_DIR=src
INCLUDE_DIR=src
BUILD_DIR=build

SOURCES=$(wildcard $(SRC_DIR)/*.c)
OBJECTS=$(SOURCES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

all: $(OBJECTS)
	$(CC) -I $(INCLUDE_DIR) $(CFLAGS) -o $(NAME) $^ 

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) -I $(INCLUDE_DIR) $(CFLAGS) -o $@ -c $<

clean:
	rm -rf $(BUILD_DIR)
	rm clippy

.PHONY: clean