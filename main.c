#include <stdlib.h>
#include <time.h>
#include <ncurses.h>
#include <string.h>
#include "spawn.h"
#include "player.h"
#include "skins.h"
#include "score.h"

#define MAX_ASTEROIDS 50
#define MAX_BULLETS 10
#define MAX_SHATLES 3

void reset_game(Player *player, Bullet bullets[], Shatle shatles[], Asteroid asteroids[], int max_asteroids, int max_shatles, int *score, int *bullets_left, int *waiting_for_shatle_wave, int *shatle_wave_active, int *shatle_to_spawn, int *shatle_sequence_index, int *last_shatle_wave_score, int *shatle_wave_done, int *shatle_spawn_timer){
    *score = 0;
    *bullets_left = MAX_BULLETS;

    for(int i = 0; i < MAX_BULLETS; i++){
        bullets[i].active = 0;
    }

    for(int i = 0; i < max_asteroids; i++){
        asteroids[i].active = 0;
    }

    for(int i = 0; i < max_shatles; i++){
        shatles[i].active = 0;
    }

    *waiting_for_shatle_wave = 0;
    *shatle_wave_active = 0;
    *shatle_to_spawn = 0;
    *shatle_sequence_index = 0;
    *last_shatle_wave_score = 0;
    *shatle_wave_done = 0;
    *shatle_spawn_timer = 0;
}

int main(){
    srand((unsigned)time(NULL));

    initscr();
    cbreak();
    noecho();
    curs_set(0);

    int max_x, max_y;
    getmaxyx(stdscr, max_y, max_x);

    int win_width = (max_x < 75) ? max_x : 75;
    int win_height = (max_y < 25) ? max_y : 25;
    int start_x = (max_x - win_width) / 2;
    int start_y = (max_y - win_height) / 2;
    
    WINDOW *win = newwin(win_height, win_width, start_y, start_x);
    box(win, 0, 0);
    int ch;
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
    Player player = {x, y, skin_width, skin_height, available_skins[skin_ship]};


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
    int bullets_left= MAX_BULLETS;
    for(int i = 0; i < MAX_BULLETS; i++){
        bullets[i].active = 0;
    }

    //menu


    while(ch != 'q'){
        werase(win);
        box(win, 0, 0);
        mvwprintw(win, (win_height / 2) - 3, (win_width / 2) - 7, "Start Game (G)");
        mvwprintw(win, (win_height / 2) - 1, (win_width / 2) - 5, "Skins (S)");
        mvwprintw(win, (win_height / 2) + 1, (win_width / 2) - 4, "Quit (Q)");

        switch (ch){
            case 's':
                while(ch != 'q'){
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
                        case '1':
                            skin_ship = 0;
                            player.shape = ship_skin1;
                            break;
                        case '2':
                            player.shape = ship_skin2;
                            skin_ship = 1;                            
                            break;
                        case '3':
                            player.shape = ship_skin3;
                            skin_ship = 2;
                            break;
                        case '4':
                            player.shape = ship_skin4;
                            skin_ship = 3;
                            break;
                    }

                    player.height = 0;
                    while(player.shape[player.height] != NULL){
                        player.height++;
                    }
                    player.width = strlen(player.shape[0]);

                    wrefresh(win);
                    ch = wgetch(win);
                }

            case 'g':
                reset_game(&player, bullets, shatles, asteroids, MAX_ASTEROIDS, MAX_SHATLES, &score, &bullets_left, &waiting_for_shatle_wave, &shatle_wave_active, &shatle_to_spawn, &shatle_sequence_index, &last_shatle_wave_score, &shatle_wave_done, &shatle_spawn_timer);

                ch = 0;

                while (ch != 'q') {
                    werase(win);
                    box(win, 0, 0);
                    mvwprintw(win, 1, 1, "Press 'q' to quit");

                    mvwprintw(win, 1, win_width - 12, "Best: %d", load_score());
                    mvwprintw(win, 2, win_width - 12, "Score: %d", score);
                    mvwprintw(win, 1, win_width / 2 - 3, "Bullets: %d", bullets_left);

                    ch = wgetch(win);

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
                            if(bullets_left > 0){
                                shoot(bullets, MAX_BULLETS, &player);
                                bullets_left--;
                            }
                            break;
                    }


                    move_bullets(bullets, MAX_BULLETS);
                    draw_bullets(win, bullets, MAX_BULLETS);
                    for(int b = 0; b < MAX_BULLETS; b++){
                        if(!bullets[b].active) continue;
                        for(int a = 0; a < MAX_ASTEROIDS; a++){
                            if(bullet_hits_asteroid(&bullets[b], &asteroids[a])){
                                score += 5;
                                bullets[b].active = 0;
                            }
                        }
                        for(int s = 0; s < MAX_SHATLES; s++){
                            int result = bullet_hits_shatle(&bullets[b], &shatles[s]);
                            if(result > 1){
                                score += result;
                            }
                        }
                    }

                    bullets_recover_timer += 10;
                    if(bullets_recover_timer >= 2500){
                        if(bullets_left < MAX_BULLETS){
                            bullets_left++;
                        }
                        bullets_recover_timer = 0;
                    }

                    if((score / 100) > last_shatle_wave_score && !shatle_wave_active){
                        waiting_for_shatle_wave = 1;
                        last_shatle_wave_score = score / 100;
                    }

                if(shatle_wave_active && shatle_to_spawn > 0){
                        shatle_spawn_timer++;
                        if(shatle_spawn_timer >= shatle_spawn_interval_ticks){
                            shatle_spawn_timer = 0;
                            int type_to_spawn = shatle_sequence[shatle_sequence_index];
                            if(shatles_spawn(shatles, MAX_SHATLES, shatle_types, num_shatle_types, type_to_spawn, win_height, win_width)){
                                shatle_sequence_index++;
                                shatle_to_spawn--;
                            }
                        }
                }

                    for(int i = 0; i < MAX_SHATLES; i++){
                        move_shatle(&shatles[i], win_width - 2);
                        draw_shatle(win, &shatles[i]);
                    }

                    if(!waiting_for_shatle_wave && !shatle_wave_active){
                        int max_active_asteroids = 15 + score / 10;
                        if(max_active_asteroids > MAX_ASTEROIDS) max_active_asteroids = MAX_ASTEROIDS;
                        int active_count = 0;
                        for(int i = 0; i < MAX_ASTEROIDS; i++){
                            if(asteroids[i].active) active_count++;
                        }
                        if(active_count < max_active_asteroids){
                            asteroids_spawn(asteroids, MAX_ASTEROIDS, asteroid_types, num_types, win_width, &spawn_timer, &spawn_interval, shatle_wave_active);
                        }
                    }
                    if(waiting_for_shatle_wave){
                        if(!any_asteroid_active(asteroids, MAX_ASTEROIDS)){
                            waiting_for_shatle_wave = 0;
                            last_shatle_wave_score = score / 100;
                            shatle_wave_active = 1;
                            shatle_to_spawn = 3;
                            shatle_sequence[0] = 1;
                            shatle_sequence[1] = 0;
                            shatle_sequence[2] = 0;
                            shuffle_sequence(shatle_sequence, 3);
                            shatle_sequence_index = 0;
                            shatle_spawn_timer = 0;
                        }
                    }
                    for(int i = 0; i < MAX_ASTEROIDS; i++){
                        if(asteroids[i].active){
                            move_down(&asteroids[i], win_height - 1);
                            draw_asteroid(win, &asteroids[i]);
                        }
                    }

                    if(shatle_wave_active && shatle_to_spawn == 0){
                        int any = 0;
                        for(int i = 0; i < MAX_SHATLES; i++){
                            if(shatles[i].active){ any = 1; break; }
                        }
                        if(!any){
                            shatle_wave_active = 0;
                            shatle_wave_done = 1;
                            spawn_timer = 0;
                        }
                    }


                    for(int i = 0; i < MAX_ASTEROIDS; i++){
                        if(check_collision(&asteroids[i], &player)){
                            mvwprintw(win, win_height / 2, win_width / 2 - 5, "GAME OVER!");
                            wrefresh(win);
                            napms(2000);
                            ch = 'q';
                            break;
                        }
                    }

                    draw_player(win, &player);

                    score_timer++;
                    if(score_timer >= 100){
                        score++;
                        score_timer = 0;
                    }

                    float difficulty_factor = 1.0 + (score / 100);
                    if(difficulty_factor > 10) difficulty_factor = 10;

                    for(int i = 0; i < MAX_ASTEROIDS; i++){
                        if(asteroids[i].active){
                            asteroids[i].speed = ((rand() % 3) + 4) / 100.0 * difficulty_factor;
                        }
                    }
                    for(int i = 0; i < MAX_SHATLES; i++){
                        if(shatles[i].active){
                            shatles[i].speed = (0.20 + ((float)(rand() % 3)) * 0.05) * difficulty_factor;
                        }
                    }
                    min_spawn_delay = 10 - (score / 10);
                    if(min_spawn_delay < 0.05) min_spawn_delay = 0.05;
                    
                    int max_active_asteroids = 20 + score / 10;
                    if(max_active_asteroids > MAX_ASTEROIDS) max_active_asteroids = MAX_ASTEROIDS;

                    wrefresh(win);

                    napms(10);
                }
                break;
            }
        wrefresh(win);
        ch = wgetch(win);
    }
    delwin(win);
    endwin();
    if(score > load_score()) save_score(score);
    return 0;
}