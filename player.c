//player.c

#include <stdlib.h>
#include <ncurses.h>
#include "skins.h"
#include "player.h"
#include "mechanics.h"

// player

void move_left(Player *p, int min_x) {
    if(p->x - 2 > min_x){
        p->x-=2;
    } else if(p->x - 1 >= min_x){
        p->x-=1;
    }
}

void move_right(Player *p, int max_x) {
    if(p->x + p->width + 2 <= max_x){
        p->x+=2; 
    } else if(p->x + p->width + 1 <= max_x){
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

void shoot(Bullet bullets[], int max_bullets, Player *p){
    for(int i = 0; i < max_bullets; i++){
        if(!bullets[i].active){
            bullets[i].x = p->x + p->width / 2;
            bullets[i].y = p->y - 1;
            bullets[i].active = 1;
            return;
        }
    }
}

void move_bullets(Bullet bullets[], int max_bullets){
    for(int i = 0; i < max_bullets; i++){
        if(bullets[i].active){
            bullets[i].y--;
            if(bullets[i].y < 1){
                bullets[i].active = 0;
            }
        }
    }
}

void draw_bullets(WINDOW *win, Bullet bullets[], int max_bullets){
    for(int i = 0; i < max_bullets; i++){
        if(bullets[i].active){
            mvwprintw(win, bullets[i].y, bullets[i].x, "|");
        }
    }
}