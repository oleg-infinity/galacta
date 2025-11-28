//player.c

#include <stdlib.h>
#include <ncurses.h>
#include "player.h"

// player

void move_left(Player *p, int min_x) {
    if(p->x > min_x+1){
        p->x-=2;
    } else {
        p->x-=1;
    }
}
void move_right(Player *p, int max_x) {
    if(p->x < max_x-1){
        p->x+=2; 
    } else {
        p->x+=1; 
    }
}
void draw_player(WINDOW *win, Player *p) {
    for(int i = 0; i < p->height; i++){
        for(int j = 0; j < p->width; j++){
            char c = p->shape[i][j];
            if(c != ' '){
                mvwprintw(win, (int)(p->y) + i, p->x + j, "%c", c);
            }
        }    
    }
}