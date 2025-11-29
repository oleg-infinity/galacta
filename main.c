#include <stdlib.h>
#include <time.h>
#include <ncurses.h>
#include <stdlib.h>
#include "spawn.h"
#include "player.h"

#define MAX_ASTEROIDS 15
#define MAX_BULLETS 10

int main(){
    srand((unsigned)time(NULL));

    initscr();
    cbreak();
    noecho();
    curs_set(0);

    int max_x, max_y;
    getmaxyx(stdscr, max_y, max_x);

    int win_width = (max_x < 50) ? max_x : 50;
    int win_height = (max_y < 20) ? max_y : 20;
    int start_x = (max_x - win_width) / 2;
    int start_y = (max_y - win_height) / 2;
    
    WINDOW *win = newwin(win_height, win_width, start_y, start_x);
    box(win, 0, 0);
    int ch;
    nodelay(win, TRUE);
    keypad(win, TRUE);
    mvwprintw(win, 1, 1, "Press 'q' to quit");
    wrefresh(win);

    int x = win_width / 2; 
    int y = win_height - 4;
    const char *shatl[] = {
        ". .",
        "|^|",
        "/0\\"
    };
    Player player = {x, y, 3, 3, shatl};

    Bullet bullets[MAX_BULLETS];

    Asteroid asteroids[MAX_ASTEROIDS];
    for(int i = 0; i < MAX_ASTEROIDS; i++) asteroids[i].active = 0;

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

    Asteroid *asteroid_types[] = {&asteroid1, &asteroid2, &asteroid3, &asteroid4, &asteroid5, &asteroid6};
    int num_types = sizeof(asteroid_types) / sizeof(asteroid_types[0]);
    
    int spawn_timer = 0;
    int spawn_interval = (rand() % 70) + 20;

    int score = 0;

    int bullets_recover_timer = 0;
    int bullets_left= MAX_BULLETS;
    for(int i = 0; i < MAX_BULLETS; i++){
        bullets[i].active = 0;
    }

    int score_timer = 0;

    while (ch != 'q') {
        werase(win);
        box(win, 0, 0);
        mvwprintw(win, 1, 1, "Press 'q' to quit");

        mvwprintw(win, 1, win_width - 10, "Score: %d", score);
        mvwprintw(win, 1, win_width / 2 - 3, "Bullets: %d", bullets_left);

        ch = wgetch(win);

        switch (ch){
            case 'a':
            case KEY_LEFT:  
            if(player.x > 1) {
                move_left(&player, 1);
                break;
            }
            case 'd':
            case KEY_RIGHT: 
            if(player.x < win_width - 4) {
                move_right(&player, win_width - 4);
                break;
            }
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
        }
        bullets_recover_timer += 10;
        if(bullets_recover_timer >= 5000){
            if(bullets_left < MAX_BULLETS){
                bullets_left++;
            }
            bullets_recover_timer = 0;
        }


        asteroids_spawn(asteroids, MAX_ASTEROIDS, asteroid_types, num_types, win_width, &spawn_timer, &spawn_interval);

        for(int i = 0; i < MAX_ASTEROIDS; i++){
            move_down(&asteroids[i], win_height - 1);
            draw_asteroid(win, &asteroids[i]);
        }


        draw_player(win, &player);
        for(int i = 0; i < MAX_ASTEROIDS; i++){
            if(check_collision(&asteroids[i], &player)){
                mvwprintw(win, win_height / 2, win_width / 2 - 5, "GAME OVER!");
                wrefresh(win);
                napms(2000);
                ch = 'q';
                break;
            }
        }

        score_timer++;
        if(score_timer >= 100){
            score++;
            score_timer = 0;
        }

        float difficulty_factor = 1.0 + (score / 100.0);
        for(int i = 0; i < MAX_ASTEROIDS; i++){
            if(asteroids[i].active){
                asteroids[i].speed = ((rand() % 3) + 1) / 100.0 * difficulty_factor;
            }
        }
        min_spawn_delay = 20 - (score / 50);
        if(min_spawn_delay < 5) min_spawn_delay = 5;

        wrefresh(win);

        napms(10);
    }
    delwin(win);
    endwin();
    return 0;
}