// spawn.h
#ifndef SPAWN_H
#define SPAWN_H

#include <ncurses.h>
#include "player.h"

typedef struct {
    int x;
    float y;
    int width;
    int height;
    const char **shape;
    float speed;
    int active;
    int row_active[5];
} Asteroid;

typedef struct {
    float x;
    float target_x;
    float y;
    int width;
    int height;
    const char **shape;
    float speed;
    int active;
    int hp;
    int bonus_score;
} Shatle;

typedef struct {
    int start;
    int end;
} SpawnZone;

int overlaps(int x, int width, Asteroid *a);

int build_spawn_zones(SpawnZone zones[], int max_zones, Asteroid asteroids[], int count, int win_width);

int choose_spawn_x(SpawnZone zones[], int zone_count, int width);

extern float min_spawn_delay;

void asteroids_spawn(Asteroid asteroids[], int max_asteroids, Asteroid **types, int num_types, int win_width, int *timer, int *interval, int shatle_wave);

void move_down(Asteroid *a, int max_y);

void draw_asteroid(WINDOW *win, Asteroid *a);

int any_asteroid_active(Asteroid asteroids[], int max);

int check_collision(Asteroid *a, Player *p);

int bullet_hits_asteroid(Bullet *b, Asteroid *a);

int shatles_spawn(Shatle shatles[], int max_shatles, Shatle **types, int num_shatle_types, int type_index, int win_height, int win_width);

void shuffle_sequence(int *seq, int n);

void move_shatle(Shatle *s, int max_x);

void draw_shatle(WINDOW *win, Shatle *s);

int bullet_hits_shatle(Bullet *b, Shatle *s);

#endif