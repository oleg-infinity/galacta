CC = gcc
CFLAGS = -Wall -Wextra
LDFLAGS = -lncurses -lpthread
SRCS = main.c spawn.c player.c skins.c score.c server.c client.c mechanics.c
OBJS = $(SRCS:.c=.o)
TARGET = game
BIN_DIR = compiled

.PHONY: all clean run

all: $(BIN_DIR)/$(TARGET)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(BIN_DIR)/$(TARGET): $(OBJS) | $(BIN_DIR)
	$(CC) $(CFLAGS) $(OBJS) $(LDFLAGS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run: $(BIN_DIR)/$(TARGET)
	./$(BIN_DIR)/$(TARGET)

clean:
	rm -f $(OBJS) $(BIN_DIR)/$(TARGET)
	rmdir $(BIN_DIR) 2>/dev/null || true