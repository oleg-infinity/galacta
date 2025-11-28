//spawn.c

#include <stdlib.h>
#include "spawn.h"
#include "player.h"

// spawn asteroid functions

int overlaps(int x, int width, Asteroid *a){
    int left_new = x;
    int right_new = x + width;
    int left_old = a->x;
    int right_old = a->x + a->width;
    return !(right_new < left_old || left_new > right_old);
}

int build_spawn_zones(SpawnZone zones[], int max_zones, Asteroid asteroids[], int count, int win_width){
    Asteroid* act[32];
    int c = 0;
    for(int i = 0; i < count; i++){
        if(asteroids[i].active){
            act[c++] = &asteroids[i];
        }
    }

    if(c == 0){
        zones[0].start = 1;
        zones[0].end = win_width - 2;
        return 1;
    }

    for(int i = 0; i < c - 1; i++){
        for(int j = i+1; j < c; j++){
            if(act[j]->x < act[i]->x){
                Asteroid *tmp = act[i];
                act[i] = act[j];
                act[j] = tmp;
            }
        }
    }
    
    int z = 0;
    int last_end = 1;

    for(int i = 0; i < c; i++){
        int left = act[i]->x - 3;
        int right = act[i]->x + act[i]->width + 2;
        if(left > last_end){
            zones[z].start = last_end;
            zones[z].end = left - 1;
            z++;
            if(z >= max_zones) return z;
        }

        last_end = right + 1;
    }
    if(last_end < win_width - 2){
        zones[z].start = last_end;
        zones[z].end = win_width - 2;
        z++;
    }
    return z;
}

int choose_spawn_x(SpawnZone zones[], int zone_count, int width){
    if(zone_count == 0) return -1;
    int z = rand() % zone_count;
    int min_x = zones[z].start;
    int max_x = zones[z].end - width;
    if(max_x < min_x) return -1;
    return (rand() % (max_x - min_x + 1)) + min_x;
}

int min_spawn_delay = 20;

void asteroids_spawn(Asteroid asteroids[], int max_asteroids, Asteroid **types, int num_types, int win_width, int *timer, int *interval){
    (*timer)++;

    if(*timer < min_spawn_delay) return;

    if(*timer < *interval) return;

    int too_close = 0;
    for(int k = 0; k < max_asteroids; k++){
        if(asteroids[k].active && asteroids[k].y < 3.0f) { too_close = 1; break; }
    }   
    if(too_close) return;

    for(int i = 0; i < max_asteroids; i++){
        if(!asteroids[i].active){
            Asteroid *tpl = types[rand() % num_types];

            SpawnZone zones[16];
            int zone_count = build_spawn_zones(zones, 16, asteroids, max_asteroids, win_width);
            int x = choose_spawn_x(zones, zone_count, tpl->width);
            if(x < 0) return;

            asteroids[i].x = x;
            asteroids[i].y = 1;
            asteroids[i].width = tpl->width;
            asteroids[i].height = tpl->height;
            asteroids[i].shape = tpl->shape;
            asteroids[i].speed = ((rand() % 3) + 1) / 100.0;
            asteroids[i].active = 1;

            for(int r = 0; r < asteroids[i].height; r++){
                asteroids[i].row_active[r] = 1;
            }

            *timer = 0;
            *interval = (rand() % 60) + 40;
            return;
        }
    }
}

void move_down(Asteroid *a, int max_y) {
    if(!a->active) return;
    a->y += a->speed;
    for(int i = 0; i < a->height; i++){
        if((int)(a->y) + i >= max_y){
            a->row_active[i] = 0;
        }
    }
    int any_active = 0;
    for(int i = 0; i < a->height; i++){
        if(a->row_active[i]) {any_active = 1; break;}
    }
    if(!any_active) a->active = 0;
}

void draw_asteroid(WINDOW *win, Asteroid *a) {
    if(!a->active) return;
    for(int i = 0; i < a->height; i++){
        if(!a->row_active[i]) continue;
        for(int j = 0; j < a->width; j++){
            char c = a->shape[i][j];
            if(c != ' '){
                mvwprintw(win, (int)(a->y) + i, a->x + j, "%c", c);
            }
        }    
    }
}

int check_collision(Asteroid *a, Player *p){
    if(!a->active) return 0;

    for(int i = 0; i < a->height; i++){
        if(!a->row_active[i]) continue;
        int row_y = (int)(a->y)+i;

        for(int j = 0; j < a->width; j++){
            if(a->shape[i][j] == ' ') continue;
            int col_x = a-> x + j;

            for(int py = 0; py < p->height; py++){
                for(int px = 0; px < p->width; px++){
                    int gx = p-> x + px;
                    int gy = p-> y + py;
                    if(col_x == gx && row_y == gy){
                        return 1;
                    }
                }
            }
        }
    }
    return 0;
}