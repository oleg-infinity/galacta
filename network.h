//network.h

#ifndef NETWORK_H
#define NETWORK_H

#define DEFAULT_PORT 9000
#define MAX_CLIENTS 2
#define MAX_SYNC_ASTEROIDS 15
#define MAX_SYNC_SHATLES 3
#define MAX_SYNC_BULLETS 20

typedef enum {
    MODE_SINGLEPLAYER,
    MODE_SERVER,
    MODE_CLIENT
} GameMode;

typedef struct {
    int active;
    int x;
    int y; 
    int width;
    int height;
    int shape_id; 
} SyncAsteroid;

typedef struct {
    int x;
    int y;
    int active;
    int shooter_id;
} SyncedBullet;

typedef struct {
    float x;
    float target_x;
    float y;
    int width;
    int height;
    float speed;
    int active;
    int hp;
    int bonus_score;
    int type_id;
} SyncShatle;

typedef struct {
    int p1_x, p1_y;
    int p2_x, p2_y;
    
    SyncAsteroid asteroids[MAX_SYNC_ASTEROIDS];
    int active_asteroid_count;
    
    SyncedBullet bullets[MAX_SYNC_BULLETS];
    int active_bullet_count;

    SyncShatle shatles[MAX_SYNC_SHATLES];
    int active_shatles_count;
    
    int score;
    int p1_bullets_left;
    int p2_bullets_left;
    int game_over; 
} GameState;

#endif
