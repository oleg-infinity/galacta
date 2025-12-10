#include "color_mode.h"

#define MAX_PAIRS 16

int color_pair_player = 1;
int color_pair_asteroid = 2;
int color_pair_shatle = 3;

void init_colors(){
    init_pair(color_pair_player, COLOR_CYAN, -1);
    init_pair(color_pair_asteroid, COLOR_YELLOW, -1);
    init_pair(color_pair_shatle, COLOR_MAGENTA, -1);
}

void set_color(ColorTarget target, short fg, short bg){
    short pair_number = 1;
    switch(target){
        case COLOR_TARGET_PLAYER:
            pair_number = color_pair_player;
            break;
        case COLOR_TARGET_ASTEROID:
            pair_number = color_pair_asteroid;
            break;
        case COLOR_TARGET_SHATLE:
            pair_number = color_pair_shatle;
            break;
    }
    init_pair(pair_number, fg, bg);        
}

void apply_color(WINDOW *win, ColorTarget target){
    short pair_number = 1;
    switch(target){
        case COLOR_TARGET_PLAYER:
            pair_number = color_pair_player;
            break;
        case COLOR_TARGET_ASTEROID:
            pair_number = color_pair_asteroid;
            break;
        case COLOR_TARGET_SHATLE:
            pair_number = color_pair_shatle;
            break;
    }
    wattron(win, COLOR_PAIR(pair_number));
}