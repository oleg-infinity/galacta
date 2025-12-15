//main.c

#include <stdlib.h>
#include <time.h>
#include <ncurses.h>
#include <string.h>
#include "spawn.h"
#include "player.h"
#include "skins.h"
#include "score.h"
#include "network.h"
#include "player_input.h"
#include "mechanics.h"


int main() {

    srand((unsigned)time(NULL));

    initscr();
    cbreak();
    noecho();
    curs_set(0);

    int max_x, max_y;
    getmaxyx(stdscr, max_y, max_x);

    int max_desired_width = 100;
    int max_desired_height = 35;

    int win_width = (max_x < max_desired_width) ? max_x : max_desired_width;
    int win_height = (max_y < max_desired_height) ? max_y : max_desired_height;

    int start_x = (max_x - win_width) / 2;
    int start_y = (max_y - win_height) / 2;

    
    WINDOW *win = newwin(win_height, win_width, start_y, start_x);
    box(win, 0, 0);
    nodelay(win, TRUE);
    keypad(win, TRUE);
    mvwprintw(win, 1, 1, "Press 'q' to quit");
    wrefresh(win);

    int skin_ship = 0;
    int skin_width = strlen(available_skins[skin_ship][0]);
    int skin_height = 0;
    int x = (win_width / 2) - (skin_width / 2); 
    int y = (win_height - 4);
    while(available_skins[skin_ship][skin_height] != NULL){
        skin_height++;
    }

    Player player = {x, y, skin_width, skin_height, available_skins[skin_ship], MAX_BULLETS, 0, 1};

    Bullet bullets[MAX_BULLETS];

    Shatle shatles[MAX_SHATLES];
    for(int i = 0; i < MAX_SHATLES; i++) shatles[i].active = 0;

    const char *big_shatle_shape[] = {
        "            >>>>>===[[ EMPEROR CORE ]]===<<<<<                   ",
        "       >>>///###___###===###___###\\\\\\<<<       _____          ",
        "<<<==///###########===###########\\\\\\==>>>     /     ===\\     ",
        "<==||<<###   [[[ DARK STAR ]]]   ###>>||==>--(          ===>--",
        "<<<==\\\\\\###########===###########///==>>>     \\_____===/      ",
        "       <<<\\\\\\###___###===###___###///>>>                      ",
        "            >>>>>===[[ ENGINE VOID ]]===<<<<<                    "
    };
    Shatle shatle2 = {0, 0, 0, 64, 7, big_shatle_shape, 0.0f, 0, 5, 40};

    const char *midlle_shatle_shape[] = {
        "-->          ",
        "-->---\\->    ",
        "-->( )<>====>",
        "-->---/->    ",
        "-->          ",
    };
    Shatle shatle1 = {0, 0, 0, 13, 5, midlle_shatle_shape, 0.0f, 0, 1, 10};

    Shatle *shatle_types[] = { &shatle1, &shatle2 };
    int num_shatle_types = sizeof(shatle_types) / sizeof(shatle_types[0]);


    Asteroid asteroids[MAX_ASTEROIDS];
    for(int i = 0; i < MAX_ASTEROIDS; i++) asteroids[i].active = 0;

    const char *smallest_rub_shape[] = {
        "|"
    };
    Asteroid asteroid8 = {0, 0, 1, 1, smallest_rub_shape, 0.0f, 0};

    const char *smallest_circle_shape[] = {
        "*"
    };
    Asteroid asteroid1 = {0, 0, 1, 1, smallest_circle_shape, 0.0f, 0};

    const char *small_circle_shape[] = {
        "@"
    };
    Asteroid asteroid2 = {0, 0, 1, 1, small_circle_shape, 0.0f, 0};

    const char *small_cube_shape[] = {
        "/&.",
        "*@'"
    };
    Asteroid asteroid3 = {0, 0, 3, 2, small_cube_shape, 0.0f, 0};

    const char *mid_circle_shape[] = {
        ".-.",
        "'-'"
    };
    Asteroid asteroid4 = {0, 0, 3, 2, mid_circle_shape, 0.0f, 0};

    const char *big_circle_shape[] = {
        "/**\\",
        "\\__/"
    };
    Asteroid asteroid5 = {0, 0, 4, 2, big_circle_shape, 0.0f, 0};

    const char *bigger_circle_shape[] = {
        " .-*-. ",
        "/     \\",
        "\\     /",
        " '*-*' "
    };
    Asteroid asteroid6 = {0, 0, 7, 4, bigger_circle_shape, 0.0f, 0};

    const char *biggest_circle_shape[] = {
        "  .--**--.  ",
        " / O  .   \\ ",
        "(        o )",
        " \\.  O    / ",
        "  '--__--'  "
    };
    Asteroid asteroid7 = {0, 0, 13, 5, biggest_circle_shape, 0.0f, 0};


    Asteroid *asteroid_types[] = {&asteroid1, &asteroid2, &asteroid3, &asteroid4, &asteroid5, &asteroid6, &asteroid7, &asteroid8};
    int num_types = sizeof(asteroid_types) / sizeof(asteroid_types[0]);


    int spawn_timer = 0;
    int spawn_interval = (rand() % 70) + 20;

    int score = 0;
    int score_timer = 0;

    int last_shatle_wave_score = 0;
    int shatle_wave_active = 0;
    int shatle_wave_done = 0;
    int shatle_to_spawn = 0;
    int shatle_spawn_interval_ticks = 150;
    int shatle_spawn_timer = 0;
    int shatle_sequence[3] = {0, 1, 0};
    int shatle_sequence_index = 0;
    int waiting_for_shatle_wave = 0;

    int bullets_recover_timer = 0;
    for(int i = 0; i < MAX_BULLETS; i++){
        bullets[i].active = 0;
    }

    int game_running = 1;
    int ch = 0;

    //menu

    int current_action = ACTION_MENU;

    while(current_action != ACTION_QUIT){      
        ch = wgetch(win);
        if(ch == ERR) ch = 0;
        switch (current_action){
            case ACTION_MENU:
                current_action = main_menu(win, win_height, win_width, ch);
                break;
            case ACTION_PLAY_SINGLE:
                while(game_running) {
                    
                    ch = wgetch(win);

                    if (ch == 'q' || ch == 'Q') {
                        if(score > load_score()) save_score(score);
                        delwin(win);
                        endwin();
                        return 0;
                    }

                    werase(win);
                    box(win, 0, 0);
                    mvwprintw(win, 1, 1, "Press 'q' to quit");

                    mvwprintw(win, 1, win_width - 12, "Best: %d", load_score());
                    mvwprintw(win, 2, win_width - 12, "Score: %d", score);
                    mvwprintw(win, 1, win_width / 2 - 3, "Bullets: %d", player.bullets_left);

                    switch (ch){
                        case 'a':
                        case KEY_LEFT:  
                            move_left(&player, 1);
                            break;
                        case 'd':
                        case KEY_RIGHT: 
                            move_right(&player, win_width - 1);
                            break;
                        case ' ':
                            if(player.bullets_left > 0){ 
                                shoot(bullets, MAX_BULLETS, &player);
                                player.bullets_left--;
                            }
                            break;
                    }

                    bullets_logics_and_movement(win, bullets, MAX_BULLETS, &score, asteroids, MAX_ASTEROIDS, shatles, MAX_SHATLES);
                    bullets_recovery_logics(&player);

                    shatles_logics(win, shatles, asteroids, MAX_ASTEROIDS, &score, &last_shatle_wave_score, &waiting_for_shatle_wave, &shatle_wave_active, &shatle_to_spawn, &shatle_sequence_index, win_height, win_width, &shatle_spawn_timer, shatle_spawn_interval_ticks, shatle_types, num_shatle_types, asteroid_types, num_types, &spawn_timer, &spawn_interval, shatle_sequence, &shatle_wave_done);
                    
                    asteroids_logics(win, asteroids, MAX_ASTEROIDS, &score, &spawn_timer, &spawn_interval, asteroid_types, num_types, win_height, win_width, &player, NULL, &game_running);

                    draw_player(win, &player);

                    if (!game_running) {
                        if(score > load_score()) save_score(score);
                        werase(win);
                        box(win, 0, 0);
                        mvwprintw(win, win_height / 2, win_width / 2 - 5, "GAME OVER!");
                        mvwprintw(win, win_height / 2 + 1, win_width / 2 - 12, "Final Score: %d", score);
                        wrefresh(win);
                        napms(3000); 
                        break; 
                    }

                    score_difficulty_logics(&score, &score_timer, asteroids, MAX_ASTEROIDS, shatles, MAX_SHATLES);
                    
                    wrefresh(win);

                    napms(16);
                }
                current_action = ACTION_MENU;
                reset_game(&player, bullets, shatles, asteroids, MAX_ASTEROIDS, MAX_SHATLES, &score, &player.bullets_left, &waiting_for_shatle_wave, &shatle_wave_active, &shatle_to_spawn, &shatle_sequence_index, &last_shatle_wave_score, &shatle_wave_done, &shatle_spawn_timer, &spawn_timer, &spawn_interval, &game_running);                
                break;
            case ACTION_SKINS:
                if (ch == 'q') {
                    current_action = ACTION_MENU;
                } else {
                    werase(win);
                    box(win, 0, 0);
                    mvwprintw(win, 1, 1, "Press 'q' to quit");
                    mvwprintw(win, (win_height / 2) - 3, (win_width / 2) - 5, "Skins Menu");

                    int skin_width = strlen(available_skins[skin_ship][0]);
                    int skin_height = 0;
                    while(available_skins[skin_ship][skin_height] != NULL){
                        skin_height++;
                    }
                    int start_x = (win_width / 2) - (skin_width / 2);
                    int start_y = (win_height / 2) - (skin_height / 2);
                    draw_skin(win, available_skins[skin_ship], skin_width, skin_height, start_y, start_x);

                    switch (ch){
                        case ' ':
                            skin_ship++;
                            if(skin_ship >= num_available_skins){
                                skin_ship = 0;
                            }
                            player.shape = available_skins[skin_ship];
                            break;
                    }

                    mvwprintw(win, (win_height / 2) + 4, (win_width / 2) - 12, "PRESS SPACE TO CHANGE SKIN");

                    player.height = 0;
                    while(player.shape[player.height] != NULL){
                        player.height++;
                    }
                    player.width = strlen(player.shape[0]);

                    wrefresh(win);
                }
                break;
            case ACTION_HOST:
                if (ch == 'q') {
                    current_action = ACTION_MENU;
                } else {
                    delwin(win); 
                    endwin();
                    
                    int result = display_host_wait_menu(win_height, win_width, skin_ship);

                    reset_game(&player, bullets, shatles, asteroids, MAX_ASTEROIDS, MAX_SHATLES, &score, &player.bullets_left, &waiting_for_shatle_wave, &shatle_wave_active, &shatle_to_spawn, &shatle_sequence_index, &last_shatle_wave_score, &shatle_wave_done, &shatle_spawn_timer, &spawn_timer, &spawn_interval, &game_running);

                    initscr();
                    cbreak();
                    noecho();
                    curs_set(0);

                    clear(); 
                    refresh();
                    
                    int max_x, max_y;
                    getmaxyx(stdscr, max_y, max_x);
                    int start_x_new = (max_x - win_width) / 2;
                    int start_y_new = (max_y - win_height) / 2;
                    
                    win = newwin(win_height, win_width, start_y_new, start_x_new); 
                    box(win, 0, 0);
                    nodelay(win, TRUE);
                    keypad(win, TRUE);

                    current_action = ACTION_MENU;
                }
                break;
            case ACTION_JOIN:
                if (ch == 'q') {
                    current_action = ACTION_MENU;  
                } else {  
                    delwin(win);
                    endwin();
                    
                    int result = start_multiplayer_menu(win_height, win_width, skin_ship);
                    
                    reset_game(&player, bullets, shatles, asteroids, MAX_ASTEROIDS, MAX_SHATLES, &score, &player.bullets_left, &waiting_for_shatle_wave, &shatle_wave_active, &shatle_to_spawn, &shatle_sequence_index, &last_shatle_wave_score, &shatle_wave_done, &shatle_spawn_timer, &spawn_timer, &spawn_interval, &game_running);

                    initscr();
                    cbreak();
                    noecho();
                    curs_set(0);

                    clear(); 
                    refresh();

                    int max_x, max_y;
                    getmaxyx(stdscr, max_y, max_x);
                    int start_x_new = (max_x - win_width) / 2;
                    int start_y_new = (max_y - win_height) / 2;
                    
                    win = newwin(win_height, win_width, start_y_new, start_x_new); 
                    box(win, 0, 0);
                    nodelay(win, TRUE);
                    keypad(win, TRUE);

                    current_action = ACTION_MENU;
                }
                break;   
        }
        wrefresh(win);
        napms(16);
    }
    delwin(win);
    endwin();
    return 0;
}