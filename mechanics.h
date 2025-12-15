#ifndef MECHANICS_H
#define MECHANICS_H

#include "player.h"
#include "player_input.h"
#include "spawn.h"
#include "network.h"

#define ACTION_MENU      0 // Залишитися в меню (або повернутися до нього)
#define ACTION_PLAY_SINGLE 1 // Запустити одиночну гру
#define ACTION_SKINS     2 // Перейти до меню скінів (якщо це окремий екран)
#define ACTION_HOST      3 // Запустити хост-сервер
#define ACTION_JOIN      4 // Запустити клієнта
#define ACTION_QUIT      5 // Вийти з програми

#define MAX_ASTEROIDS 50
#define MAX_BULLETS 10
#define MAX_SHATLES 3

int run_server_game(int client_fd, int server_skin_index);
int run_client_game(int sock, int client_skin_index);
int run_client_connect(const char* ip, int port);

void get_local_ip(char *ip_buffer, size_t buffer_size);

int display_host_wait_menu(int height, int width, int skin_ship);

int start_multiplayer_menu(int height, int width, int skin_ship);

int display_lobby(int sock, int win_height, int win_width, const char *server_ip);

int display_server_lobby(int win_height, int win_width);

void reset_game(Player *player, Bullet bullets[], Shatle shatles[], Asteroid asteroids[], int max_asteroids, int max_shatles, int *score, int *bullets_left, int *waiting_for_shatle_wave, int *shatle_wave_active, int *shatle_to_spawn, int *shatle_sequence_index, int *last_shatle_wave_score, int *shatle_wave_done, int *shatle_spawn_timer, int *spawn_timer, int *spawn_interval, int *game_running);

void bullets_logics(WINDOW *win, Bullet bullets[], int max_bullets, int *bullets_left, int *bullets_recover_timer, int *score, Asteroid asteroids[], int max_asteroids, Shatle shatles[], int max_shatles);

void bullets_logics_and_movement(WINDOW *win, Bullet bullets[], int max_bullets, int *score, Asteroid asteroids[], int max_asteroids, Shatle shatles[], int max_shatles);

void bullets_recovery_logics(Player *p);

void shatles_logics(WINDOW *win, Shatle shatles[], Asteroid asteroids[], int max_asteroids, int *score, int *last_shatle_wave_score, int *waiting_for_shatle_wave, int *shatle_wave_active, int *shatle_to_spawn, int *shatle_sequence_index, int win_height, int win_width, int *shatle_spawn_timer, int shatle_spawn_interval_ticks, Shatle *shatle_types[], int num_shatle_types, Asteroid *asteroid_types[], int num_types, int *spawn_timer, int *spawn_interval, int shatle_sequence[], int *shatle_wave_done);

void asteroids_logics(WINDOW *win, Asteroid asteroids[], int max_asteroids, int *score, int *spawn_timer, int *spawn_interval, Asteroid *asteroid_types[], int num_types, int win_height, int win_width, Player *p1, Player *p2, int *game_running_flag);

void score_difficulty_logics(int *score, int *score_timer, Asteroid asteroids[], int max_asteroids, Shatle shatles[], int max_shatles);

int main_menu(WINDOW *win, int win_height, int win_width, int ch);

#endif