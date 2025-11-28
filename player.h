#ifndef PLAYER_H
#define PLAYER_H

#include <ncurses.h>

typedef struct {
    int x;
    int y;
    int width;
    int height;
    const char **shape;
    int symbol;
} Player;

void move_left(Player *p, int min_x);

void move_right(Player *p, int max_x);

void draw_player(WINDOW *win, Player *p);

#endif