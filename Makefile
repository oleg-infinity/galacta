CC = gcc
CFLAGS = -Wall -lncurses
OBJDIR = compiled
SRC = main.c spawn.c player.c skins.c score.c
OBJ = $(patsubst %.c,$(OBJDIR)/%.o,$(SRC))
OUT = game

all: $(OUT)

$(OBJDIR)/%.o: %.c | $(OBJDIR)
	$(CC) -c $< -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(OUT): $(OBJ)
	$(CC) $(OBJ) -lncurses -o $(OUT)

clean:
	rm -rf $(OBJDIR) $(OUT)

run: $(OUT)
	./$(OUT)
