#ifndef PLAYER_H
#define PLAYER_H

#include <ncurses.h>

typedef struct {
    int x;
    int y;
    int active;
} Bullet;

typedef struct {
    int x;
    int y;
    int width;
    int height;
    const char **shape;
    int bullets_left;
    int bullets_recover_timer;
    int active;
} Player;

void move_left(Player *p, int min_x);

void move_right(Player *p, int max_x);

void draw_player(WINDOW *win, Player *p);

void shoot(Bullet bullets[], int max_bullets, Player *p);

void move_bullets(Bullet bullets[], int max_bullets);

void draw_bullets(WINDOW *win, Bullet bullets[], int max_bullets);

#endif