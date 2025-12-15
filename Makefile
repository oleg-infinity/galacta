CC = gcc
CFLAGS = -Wall -Wextra -std=c99
LDFLAGS = -lncurses -lpthread
TARGET = game
SRCS = main.c spawn.c player.c skins.c score.c server.c client.c mechanics.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

main.o: main.h spawn.h player.h skins.h score.h network.h player_input.h mechanics.h
spawn.o: spawn.h player.h
player.o: player.h
skins.o: skins.h
score.o: score.h
server.o: network.h player.h mechanics.h
client.o: network.h player.h mechanics.h
mechanics.o: player.h spawn.h

clean:
	rm -f $(OBJS) $(TARGET)