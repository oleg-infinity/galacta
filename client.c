//client.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "network.h"
#include "player.h"
#include "player_input.h"
#include <ncurses.h>
#include "skins.h"
#include "spawn.h"
#include "score.h"
#include "mechanics.h"

int run_client_connect(const char* ip, int port) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) { 
        perror("socket"); 
        return 1; 
    }

    struct sockaddr_in serv;
    serv.sin_family = AF_INET;
    serv.sin_port = htons(port);
    
    if (inet_pton(AF_INET, ip, &serv.sin_addr) <= 0) {
        fprintf(stderr, "Invalid address/ Address not supported\n");
        close(sock);
        return 1;
    }

    printf("[CLIENT] Attempting to connect to: %s:%d\n", ip, port);
    
    if (connect(sock, (struct sockaddr*)&serv, sizeof(serv)) < 0) { 
        perror("[CLIENT] Connection failed"); 
        close(sock);
        return 1; 
    }

    printf("[CLIENT] Connected! Starting game...\n");
    return run_client_game(sock, 0); 
}


int run_client_game(int sock, int client_skin_index) {

    initscr();
    cbreak();
    noecho();
    curs_set(0);
    int max_x, max_y;
    getmaxyx(stdscr, max_y, max_x);
    int desired_max_width = 100;
    int desired_max_height = 35;

    int win_width = (max_x < desired_max_width ) ? max_x : desired_max_width ;
    int win_height = (max_y < desired_max_height ) ? max_y : desired_max_height ;
    
    if (win_width < 10 || win_height < 5) {
        endwin();
        fprintf(stderr, "Terminal is too small to display the window.\n");
        exit(1);
    }
    int start_x = (max_x - win_width) / 2;
    int start_y = (max_y - win_height) / 2;
    
    WINDOW *win = newwin(win_height, win_width, start_y, start_x);
    box(win, 0, 0);
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

    int p2_skin_ship = client_skin_index;
    int p2_skin_height = 0;
    while(available_skins[p2_skin_ship][p2_skin_height] != NULL){ p2_skin_height++; }
    int p2_skin_width = strlen(available_skins[p2_skin_ship][0]);

    int p1_skin_ship = 0;
    int p1_skin_height = 0;
    while(available_skins[p1_skin_ship][p1_skin_height] != NULL){ p1_skin_height++; }
    int p1_skin_width = strlen(available_skins[p1_skin_ship][0]);

    int x1 = win_width / 2 - 2;
    int y1 = win_height - 4;

    int x2 = win_width / 2 - 2;
    int y2 = win_height - 7;

    Player p1 = {x1, y1, p1_skin_width, p1_skin_height, available_skins[p1_skin_ship], MAX_BULLETS, 0, 1}; 
    Player p2 = {x2, y2, p2_skin_width, p2_skin_height, available_skins[p2_skin_ship], MAX_BULLETS, 0, 1};  
       
    char skin_sync_buf[32];
    snprintf(skin_sync_buf, sizeof(skin_sync_buf), "SKIN %d", client_skin_index); 
    send(sock, skin_sync_buf, strlen(skin_sync_buf), 0);

    char host_skin_recv_buf[32];
    int host_skin_index_new = 0; 

    if (recv(sock, host_skin_recv_buf, sizeof(host_skin_recv_buf) - 1, 0) > 0) {
        host_skin_recv_buf[31] = '\0';
        if (sscanf(host_skin_recv_buf, "HOST_SKIN %d", &host_skin_index_new) == 1) {
            
            if (host_skin_index_new >= 0 && host_skin_index_new < num_available_skins) { 
                p1_skin_ship = host_skin_index_new;
            }
        }
    }
    
    p1_skin_height = 0;
    while(available_skins[p1_skin_ship][p1_skin_height] != NULL){ p1_skin_height++; }
    p1_skin_width = strlen(available_skins[p1_skin_ship][0]);
    
    p1.width = p1_skin_width;
    p1.height = p1_skin_height;
    p1.shape = available_skins[p1_skin_ship];
    p1.x = win_width / 2 - (p1_skin_width / 2); 

    char buf[64];
    int ch; 

    int score = 0;

    int bullets_left= MAX_BULLETS;
    for(int i = 0; i < MAX_BULLETS; i++){
        bullets[i].active = 0;
    }
    
    while (1) {
        ch = wgetch(win); 

        if (ch == 'q' || ch == 'Q') {
            if(score > load_score()) save_score(score);
            delwin(win);
            endwin();
            return ACTION_MENU;
        }
        
        if (ch != ERR) {
            snprintf(buf, sizeof(buf), "KEY %c", ch); 
            send(sock, buf, strlen(buf), 0);
        }
        
        GameState state;
        int n = recv(sock, &state, sizeof(state), MSG_DONTWAIT);
        
        if (n == sizeof(state)) { 
            werase(win);
            box(win, 0, 0);
            
            if (state.game_over) {
                if(score > load_score()) save_score(score);
                box(win, 0, 0);
                mvwprintw(win, win_height / 2, win_width / 2 - 5, "GAME OVER!");
                mvwprintw(win, win_height / 2 + 1, win_width / 2 - 8, "Final Score: %d", score);
                wrefresh(win);
                napms(3000); 
                break; 
            }
            
            p1.x = state.p1_x;
            p1.y = state.p1_y;
            p2.x = state.p2_x;
            p2.y = state.p2_y;

            bullets_left = state.p2_bullets_left;
            score = state.score;
            

            for (int i = 0; i < MAX_SYNC_BULLETS; i++) {
                if (state.bullets[i].active) {
                    mvwprintw(win, state.bullets[i].y, state.bullets[i].x, "|"); 
                }
            }

            for (int i = 0; i < state.active_asteroid_count; i++) {
                int id = state.asteroids[i].shape_id;
                
                if (id >= 0 && id < num_types) { 
                    Asteroid *tpl = asteroid_types[id]; 
                    
                    Asteroid temp_a = {0}; 
                    temp_a.x = state.asteroids[i].x;
                    temp_a.y = (float)state.asteroids[i].y; 
                    temp_a.width = tpl->width;
                    temp_a.height = tpl->height;
                    temp_a.shape = tpl->shape;

                    temp_a.active = 1;
                    for(int r = 0; r < temp_a.height; r++) { temp_a.row_active[r] = 1; }

                    draw_asteroid(win, &temp_a);
                }
            }

            for (int i = 0; i < state.active_shatles_count; i++) {
                SyncShatle *ss = &state.shatles[i];
                
                if (ss->type_id >= 0 && ss->type_id < num_shatle_types) {
                    Shatle temp_s = {0};
                    temp_s.x = ss->x;
                    temp_s.y = ss->y;
                    temp_s.width = ss->width;
                    temp_s.height = ss->height;
                    temp_s.active = 1;
                    
                    temp_s.shape = shatle_types[ss->type_id]->shape;
                    
                    draw_shatle(win, &temp_s);
                }
            }

            draw_player(win, &p1); 
            draw_player(win, &p2); 
            
            mvwprintw(win, 1, 1, "Press 'q' to quit");

            mvwprintw(win, 1, win_width - 12, "Best: %d", load_score());
            mvwprintw(win, 2, win_width - 12, "Score: %d", score);
            mvwprintw(win, 1, win_width / 2 - 3, "Bullets: %d", bullets_left);

            wrefresh(win);
        }
    }
    delwin(win);
    endwin();
    close(sock);
    return ACTION_MENU;
}

void client_send_player(int sock, Player *p2) {
    char buf[64];
    sprintf(buf, "P2 %d %d\n", p2->x, p2->y);
    send(sock, buf, strlen(buf), 0);
}

void client_receive_player(int sock, Player *p1) {
    char buf[64];
    int n = recv(sock, buf, sizeof(buf)-1, MSG_DONTWAIT);
    if (n > 0) {
        buf[n] = 0;
        if (strncmp(buf, "P1", 2) == 0) {
            sscanf(buf, "P1 %d %d", &p1->x, &p1->y);
        }
    }
}
