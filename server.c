//server.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "network.h"
#include "player.h"
#include "player_input.h"
#include <ncurses.h>
#include "skins.h"
#include "mechanics.h"
#include <time.h>
#include "spawn.h"
#include "score.h"

int run_server_game(int client_fd, int server_skin_index ){
    // === НОВИЙ БЛОК: ОЧІКУВАННЯ В ЛОБІ ===
    // 1. Отримуємо розміри терміналу
    initscr(); // Тимчасово ініціалізуємо ncurses для отримання розміру
    endwin();  // Одразу закриваємо
    
    int max_x, max_y;
    getmaxyx(stdscr, max_y, max_x);
    endwin();

    // --- 1. ІНІЦІАЛІЗАЦІЯ Ncurses та WINDOW ---
    initscr();
    cbreak();
    noecho();
    curs_set(0);
    int desired_max_width = 100;
    int desired_max_height = 35;

    int win_width = (max_x < desired_max_width ) ? max_x : desired_max_width ;
    int win_height = (max_y < desired_max_height ) ? max_y : desired_max_height ;
    
    // Перевірка на мінімальний розмір, щоб не було newwin(0, X, Y, Z)
    if (win_width < 10 || win_height < 5) {
        endwin();
        fprintf(stderr, "Terminal is too small to display the window.\n");
        exit(1);
    }

    int start_x = (max_x - win_width) / 2;
    int start_y = (max_y - win_height) / 2;

    srand((unsigned)time(NULL));
    
    WINDOW *win = newwin(win_height, win_width, start_y, start_x);
    box(win, 0, 0);
    mvwprintw(win, 1, 1, "Press 'q' to quit");
    wrefresh(win);
    nodelay(win, TRUE);
    keypad(win, TRUE);

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
reset_game
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
    // --------------------------------------------------

    // --- 2. ІНІЦІАЛІЗАЦІЯ ГРАВЦІВ ---
    // Нам потрібно визначити параметри скіна

    int p1_skin_ship = server_skin_index;
    int p1_skin_height = 0;
    while(available_skins[p1_skin_ship][p1_skin_height] != NULL){ p1_skin_height++; }
    int p1_skin_width = strlen(available_skins[p1_skin_ship][0]);


    int p2_skin_ship = 0;
    int p2_skin_height = 0;
    while(available_skins[p2_skin_ship][p2_skin_height] != NULL){ p2_skin_height++; }
    int p2_skin_width = strlen(available_skins[p2_skin_ship][0]);

    int x1 = win_width / 2 - 2;
    int y1 = win_height - 4;

    int x2 = win_width / 2 - 2;
    int y2 = win_height - 7;

    Player p1 = {x1, y1, p1_skin_width, p1_skin_height, available_skins[p1_skin_ship], MAX_BULLETS, 0, 1}; // Серверний гравець
    Player p2 = {x2, y2, p2_skin_width, p2_skin_height, available_skins[p2_skin_ship], MAX_BULLETS, 0, 1};  // Клієнтський гравець
    
    char skin_recv_buf[32];
    int client_skin_index_new = 0;
    if (recv(client_fd, skin_recv_buf, sizeof(skin_recv_buf) - 1, 0) > 0) {
        skin_recv_buf[31] = '\0';
        if (sscanf(skin_recv_buf, "SKIN %d", &client_skin_index_new) == 1) {
            if (client_skin_index_new >= 0 && client_skin_index_new < num_available_skins) { 
                p2_skin_ship = client_skin_index_new;
            }
        }
    }

    p2_skin_height = 0;
    while(available_skins[p2_skin_ship][p2_skin_height] != NULL){ p2_skin_height++; }
    p2_skin_width = strlen(available_skins[p2_skin_ship][0]);
    
    p2.width = p2_skin_width;
    p2.height = p2_skin_height;
    p2.shape = available_skins[p2_skin_ship];
    p2.x = win_width / 2 - (p2_skin_width / 2);

    char host_skin_sync_buf[32];
    // Формат: HOST_SKIN <індекс_скіна_хоста>
    snprintf(host_skin_sync_buf, sizeof(host_skin_sync_buf), "HOST_SKIN %d", server_skin_index); 
    send(client_fd, host_skin_sync_buf, strlen(host_skin_sync_buf), 0);

    reset_game(NULL, bullets, shatles, asteroids, MAX_ASTEROIDS, MAX_SHATLES, &score, &p1.bullets_left, &waiting_for_shatle_wave, &shatle_wave_active, &shatle_to_spawn, &shatle_sequence_index, &last_shatle_wave_score, &shatle_wave_done, &shatle_spawn_timer, &spawn_timer, &spawn_interval, &game_running);
    
    p2.bullets_left = p1.bullets_left;
    
    char buf[64];
    int ch;
    
    while (game_running) {
        ch = wgetch(win);

        if (ch == 'q' || ch == 'Q') {
            delwin(win);
            endwin();
            return 0;
        }

        if (ch != ERR) { 
             input_move_player(&p1, ch, win_width, bullets);  // Рух P1 виконується тут!
        }

        // B. ОБРОБКА ВВОДУ КЛІЄНТА (P2)
        // Сервер отримує ввід і рухає p2
        int n = recv(client_fd, buf, sizeof(buf)-1, MSG_DONTWAIT);
        if (n > 0) {
            buf[n] = 0;
            if (strncmp(buf, "KEY ", 4) == 0) {
                char client_ch = buf[4];
                input_move_player(&p2, client_ch, win_width, bullets); // Рух P2 виконується тут!
            }
        }

        werase(win);
        box(win, 0, 0);
        
        // C. ОНОВЛЕННЯ СВІТУ (TODO: Астероїди, Кулі, Зіткнення - лише на Сервері)
        // ... (Тут буде ваша ігрова логіка) ...
        bullets_logics_and_movement(win, bullets, MAX_BULLETS, &score, asteroids, MAX_ASTEROIDS, shatles, MAX_SHATLES);        bullets_recovery_logics(&p1);        
        bullets_recovery_logics(&p2);
        
        shatles_logics(win, shatles, asteroids, MAX_ASTEROIDS, &score, &last_shatle_wave_score, &waiting_for_shatle_wave, &shatle_wave_active, &shatle_to_spawn, &shatle_sequence_index, win_height, win_width, &shatle_spawn_timer, shatle_spawn_interval_ticks, shatle_types, num_shatle_types, asteroid_types, num_types, &spawn_timer, &spawn_interval, shatle_sequence, &shatle_wave_done);
        asteroids_logics(win, asteroids, MAX_ASTEROIDS, &score, &spawn_timer, &spawn_interval, asteroid_types, num_types, win_height, win_width, &p1, &p2, &game_running);
          
        score_difficulty_logics(&score, &score_timer, asteroids, MAX_ASTEROIDS, shatles, MAX_SHATLES);

        // D. НАДСИЛАННЯ СТАНУ ОБОХ ГРАВЦІВ КЛІЄНТУ
        GameState state = {0};
        // 1. Заповнення стану гравців
        state.p1_x = p1.x;
        state.p1_y = p1.y;
        state.p2_x = p2.x;
        state.p2_y = p2.y;
        state.score = score;
        state.game_over = !game_running; // Треба реалізувати перевірку Game Over

        state.p1_bullets_left = p1.bullets_left;  
        state.p2_bullets_left = p2.bullets_left;  

        // 2. Заповнення стану астероїдів
        state.active_asteroid_count = 0;
        for (int i = 0; i < MAX_ASTEROIDS; i++) {
            if (asteroids[i].active && state.active_asteroid_count < MAX_SYNC_ASTEROIDS) {
                state.asteroids[state.active_asteroid_count].active = 1;
                state.asteroids[state.active_asteroid_count].x = asteroids[i].x;
                state.asteroids[state.active_asteroid_count].y = (int)asteroids[i].y; 
                
                // !!! ТЕПЕР МИ ЗБЕРІГАЄМО ID, ЯКИЙ СПАВН ЗБЕРІГ !!!
                state.asteroids[state.active_asteroid_count].shape_id = asteroids[i].type_id; // <-- Готово!
                
                state.active_asteroid_count++;
            }
        }

        state.active_bullet_count = 0;
        for (int i = 0; i < MAX_BULLETS; i++) {
            if (bullets[i].active && state.active_bullet_count < MAX_SYNC_BULLETS) {
                state.bullets[state.active_bullet_count].active = 1;
                state.bullets[state.active_bullet_count].x = bullets[i].x;
                state.bullets[state.active_bullet_count].y = bullets[i].y; 
                
                // Якщо кулі мають різні форми/типи, тут також потрібен type_id!
                
                state.active_bullet_count++;
            }
        }

        // 3. Надсилання сирих бінарних даних
        send(client_fd, &state, sizeof(state), 0);

        if(!game_running) {
            mvwprintw(win, win_height / 2, win_width / 2 - 5, "GAME OVER!");
            wrefresh(win);
            napms(2000);
            break;
        }
        
        // E. МАЛЮВАННЯ (Сервер бачить обидва кораблі)
        draw_player(win, &p1); 
        draw_player(win, &p2); 
        
        mvwprintw(win, 1, 1, "Press 'q' to quit");

        mvwprintw(win, 1, win_width - 12, "Best: %d", load_score());
        mvwprintw(win, 2, win_width - 12, "Score: %d", score);
        mvwprintw(win, 1, win_width / 2 - 3, "Bullets: %d", p1.bullets_left);

        wrefresh(win);
        napms(16); 
    }

    // --- 4. ОЧИСТКА ---
    delwin(win);
    endwin();
    close(client_fd);
    return ACTION_MENU;
}

void server_send_player(int sock, Player *p) {
    char buf[64];
    sprintf(buf, "P1 %d %d\n", p->x, p->y);
    send(sock, buf, strlen(buf), 0);
}

void server_receive_player(int sock, Player *p2) {
    char buf[64];
    int n = recv(sock, buf, sizeof(buf)-1, MSG_DONTWAIT);
    if (n > 0) {
        buf[n] = 0;
        if (strncmp(buf, "P2", 2) == 0) {
            sscanf(buf, "P2 %d %d", &p2->x, &p2->y);
        }
    }
}

