// spawn.h
#ifndef SKINS_H
#define SKINS_H

#include <ncurses.h>
#include "skins.h"

extern const char *ship_skin1[];
extern const char *ship_skin2[];
extern const char *ship_skin3[];
extern const char *ship_skin4[];

extern const char **available_skins[];
extern int num_available_skins;

void draw_skin(WINDOW *win, const char **skin, int skin_width, int skin_height, int start_y, int start_x);

#endif