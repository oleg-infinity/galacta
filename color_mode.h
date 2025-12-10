#ifndef COLOR_MODE_H
#define COLOR_MODE_H

#include <ncurses.h>

typedef enum {
    COLOR_TARGET_PLAYER,
    COLOR_TARGET_ASTEROID,
    COLOR_TARGET_SHATLE,
} ColorTarget;

typedef struct {
    short r, g, b;
} RGBColor;

extern int color_pair_player;
extern int color_pair_asteroid;
extern int color_pair_shatle;

void init_colors();
void set_color(ColorTarget target, short fg, short bg);
void apply_color(WINDOW *win, ColorTarget target);

#endif