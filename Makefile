CC = gcc
CFLAGS = -Wno-unused-parameter -Wno-unused-variable -Wno-missing-field-initializers
LDFLAGS = -lncurses -lpthread
SRCS = main.c spawn.c player.c skins.c score.c server.c client.c mechanics.c
BIN_DIR = compiled
OBJS = $(addprefix $(BIN_DIR)/, $(SRCS:.c=.o))
TARGET = game

.PHONY: all clean run

all: $(BIN_DIR)/$(TARGET)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(BIN_DIR)/$(TARGET): $(OBJS) | $(BIN_DIR)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

$(BIN_DIR)/%.o: %.c | $(BIN_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

run: $(BIN_DIR)/$(TARGET)
	./$(BIN_DIR)/$(TARGET)

clean:
	rm -f $(OBJS) $(BIN_DIR)/$(TARGET)
	rmdir $(BIN_DIR) 2>/dev/null || true