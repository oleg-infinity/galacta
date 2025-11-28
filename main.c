#include <stdlib.h>
#include <time.h>
#include <ncurses.h>
#include <stdlib.h>
#include "spawn.h"

#define MAX_ASTEROIDS 15

typedef struct {
    int x;
    int y;
    int symbol;
} Player;

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
    mvwprintw(win, p->y, p->x, "%c", p->symbol);
}

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
    int y = win_height - 2;
    Player player = {x, y, '^'};

    Asteroid asteroids[MAX_ASTEROIDS];
    for(int i = 0; i < MAX_ASTEROIDS; i++) asteroids[i].active = 0;

    const char *smallest_circle_shape[] = {
        "@"
    };
    Asteroid asteroid1 = {0, 0, 1, 1, smallest_circle_shape, 0.0f, 0};

    const char *small_circle_shape[] = {
        ".-.",
        "'-'"
    };
    Asteroid asteroid2 = {0, 0, 3, 2, small_circle_shape, 0.0f, 0};

    const char *big_circle_shape[] = {
        "/**\\",
        "\\__/"
    };
    Asteroid asteroid3 = {0, 0, 4, 2, big_circle_shape, 0.0f, 0};

    const char *bigger_circle_shape[] = {
        " .-*-. ",
        "<|   |>",
        " '*-*' "
    };
    Asteroid asteroid4 = {0, 0, 7, 3, bigger_circle_shape, 0.0f, 0};

    Asteroid *asteroid_types[] = {&asteroid1, &asteroid2, &asteroid3, &asteroid4};
    int num_types = sizeof(asteroid_types) / sizeof(asteroid_types[0]);
    
    int spawn_timer = 0;
    int spawn_interval = (rand() % 70) + 20;

    while (ch != 'q') {
        werase(win);
        box(win, 0, 0);
        mvwprintw(win, 1, 1, "Press 'q' to quit");
        ch = wgetch(win);

        if(ch == 'a' && player.x > 1) move_left(&player, 1);
        if(ch == 'd' && player.x < win_width - 2) move_right(&player, win_width - 2);

        switch (ch) {
            case KEY_LEFT:  
            if(player.x > 1) {
                move_left(&player, 1);
                break;
            }
            case KEY_RIGHT: 
            if(player.x < win_width - 2) {
                move_right(&player, win_width - 2);
                break;
            }
        }

        asteroids_spawn(asteroids, MAX_ASTEROIDS, asteroid_types, num_types, win_width, &spawn_timer, &spawn_interval);

        for(int i = 0; i < MAX_ASTEROIDS; i++){
            move_down(&asteroids[i], win_height - 1);
            draw_asteroid(win, &asteroids[i]);
        }

        draw_player(win, &player);
        wrefresh(win);

        napms(10);
    }
    delwin(win);
    endwin();
    return 0;
}