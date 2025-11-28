// spawn.h
#ifndef SPAWN_H
#define SPAWN_H

#include <ncurses.h>

typedef struct {
    int x;
    float y;
    int width;
    int height;
    const char **shape;
    float speed;
    int active;
} Asteroid;

typedef struct {
    int start;
    int end;
} SpawnZone;

int overlaps(int x, int width, Asteroid *a);

int biuld_spawn_zones(SpawnZone zones[], int max_zones, Asteroid asteroids[], int count, int win_width);

int choose_spawn_x(SpawnZone zones[], int zone_count, int width);

void asteroids_spawn(Asteroid asteroids[], int max_asteroids, Asteroid **types, int num_types, int win_width, int *timer, int *interval);

void move_down(Asteroid *a, int max_y);

void draw_asteroid(WINDOW *win, Asteroid *a);

#endif