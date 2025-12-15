//mechanics.c

#include <ncurses.h>
#include "player.h"
#include "player_input.h"
#include "mechanics.h"
#include "spawn.h"
#include "network.h"
#include <ifaddrs.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <errno.h> 
#include <sys/time.h>
#include <netdb.h>

int run_server_game(int client_fd, int server_skin_index);
int run_client_game(int sock, int client_skin_index);
int run_client_connect(const char* ip, int port);

void get_local_ip(char *ip_buffer, size_t buffer_size) {
    struct ifaddrs *ifaddr, *ifa;
    int family, s;
    char host[NI_MAXHOST];
    
    // Встановлюємо IP за замовчуванням
    strncpy(ip_buffer, "127.0.0.1", buffer_size);

    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        return;
    }

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL) continue;
        
        family = ifa->ifa_addr->sa_family;

        if (family == AF_INET) {
            s = getnameinfo(ifa->ifa_addr,
                            (family == AF_INET) ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6),
                            host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
            if (s != 0) {
                printf("getnameinfo() failed: %s\n", gai_strerror(s));
                continue;
            }
            // Шукаємо першу робочу IP-адресу, відмінну від локальної петлі (127.0.0.1)
            if (strcmp(host, "127.0.0.1") != 0 && strncmp(ifa->ifa_name, "lo", 2) != 0) {
                strncpy(ip_buffer, host, buffer_size);
                break; 
            }
        }
    }

    freeifaddrs(ifaddr);
}

int display_host_wait_menu(int height, int width, int server_index_skin) {
    WINDOW *wait_win;
    char ip_buffer[20];
    int server_fd, client_fd = -1;
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    int ch;
    
    // --- 1. ОТРИМАННЯ IP ТА СТВОРЕННЯ СОКЕТА ---
    get_local_ip(ip_buffer, sizeof(ip_buffer)); 

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) { perror("socket"); return -1; }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(DEFAULT_PORT);

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind"); close(server_fd); return -1;
    }

    if (listen(server_fd, 1) < 0) { perror("listen"); close(server_fd); return -1; }

    initscr();
    cbreak();
    noecho();
    curs_set(0); 

    int start_x = (width - 60) / 2;
    int start_y = (height - 15) / 2;
    wait_win = newwin(15, 60, start_y, start_x);
    box(wait_win, 0, 0);
    keypad(wait_win, TRUE);
    nodelay(wait_win, TRUE); // Неблокуючий ввід для оновлення

    mvwprintw(wait_win, 1, 2, "=== HOSTING GAME ===");
    mvwprintw(wait_win, 3, 2, "Server IP:");
    mvwprintw(wait_win, 4, 15, ">>> %s <<<", ip_buffer);
    mvwprintw(wait_win, 6, 2, "PORT: %d", DEFAULT_PORT);
    mvwprintw(wait_win, 8, 2, "STATUS: Waiting for client connection...");
    mvwprintw(wait_win, 10, 2, "Press 'Q' to Cancel hosting.");
    wrefresh(wait_win);

    int flags = fcntl(server_fd, F_GETFL, 0);
    fcntl(server_fd, F_SETFL, flags | O_NONBLOCK);
    
    while(1) {
        // A. Перевірка вводу (Q для скасування)
        ch = wgetch(wait_win);
        if (ch == 'q' || ch == 'Q') {
            client_fd = -1; 
            break;
        }

        // B. Спроба прийняти підключення
        client_fd = accept(server_fd, (struct sockaddr*)&addr, &addrlen);
        if (client_fd >= 0) {
            break; // Клієнт підключився!
        } else if (errno != EWOULDBLOCK && errno != EAGAIN && client_fd == -1) {
             // Інша помилка accept
             perror("accept");
             break;
        }

        napms(100); // Затримка для зменшення навантаження
    }

    delwin(wait_win);
    endwin(); 
    close(server_fd); // Серверний слухаючий сокет більше не потрібен

    // --- 4. ЗАПУСК ЛОБІ ТА ГРИ ---
    if (client_fd >= 0) {
        printf("[SERVER] Client connected! Entering Lobby...\n");
        // Запускаємо лобі сервера (тепер у блокуючому режимі)
        int ready = display_server_lobby(height, width); 
        
        if (ready) {
            // run_server тепер має приймати client_fd, а не порт
            return run_server_game(client_fd, server_index_skin); 
        } else {
            // Закриваємо з'єднання, якщо хост скасував у лобі
            close(client_fd);
            return 0;
        }
    }
    return 0; // Скасовано або помилка
}

int start_multiplayer_menu(int height, int width, int skin_ship) {
    WINDOW *input_win;
    char ip_buffer[16] = ""; 
    int ch;

    // --- 1. Ввід IP (Виглядає правильно) ---
    initscr();
    cbreak();
    noecho();
    curs_set(1); 

    // Визначення розмірів вікна (якщо вони були передані, використовуємо їх)
    int max_x = width; // Припустимо, що width - це max_x
    int max_y = height; // Припустимо, що height - це max_y

    input_win = newwin(5, 40, (max_y / 2) - 2, (max_x / 2) - 20);
    box(input_win, 0, 0);
    mvwprintw(input_win, 1, 1, "Enter Server IP (e.g., 192.168.1.100):");
    mvwprintw(input_win, 3, 1, "> ");
    wrefresh(input_win);

    int ip_index = 0;
    while(1) {
        ch = wgetch(input_win);
        if (ch == 'q' || ch == 'Q') {
            delwin(input_win);
            endwin();
            return 0; // Повертаємо 0, щоб головне меню знало про скасування
        }
        if (ch == '\n' || ch == KEY_ENTER) {
            ip_buffer[ip_index] = '\0';
            break;
        } else if (ch == KEY_BACKSPACE || ch == 127) { // Обробка Backspace
            if (ip_index > 0) {
                ip_index--;
                ip_buffer[ip_index] = ' '; // Видаляємо символ
            }
        } else if (ip_index < 15 && (ch == '.' || (ch >= '0' && ch <= '9'))) {
            ip_buffer[ip_index] = (char)ch;
            ip_index++;
        }
        
        mvwprintw(input_win, 3, 3, "%-15s", ip_buffer); // Вивести буфер та очистити залишок
        wrefresh(input_win);
    }

    delwin(input_win);
    endwin(); 
    
    // --- 2. Спроба підключення без ncurses ---
    if (ip_buffer[0] != '\0') {
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) { perror("socket"); return -1; }

        struct sockaddr_in serv;
        serv.sin_family = AF_INET;
        serv.sin_port = htons(DEFAULT_PORT);
        inet_pton(AF_INET, ip_buffer, &serv.sin_addr);

        printf("[CLIENT] Attempting to connect to: %s\n", ip_buffer);

        if (connect(sock, (struct sockaddr*)&serv, sizeof(serv)) < 0) { 
            perror("[CLIENT] Connection failed"); 
            return -1; // Помилка підключення
        }
        
        // --- 3. ЛОБІ ---
        // Якщо підключення вдале, викликаємо лобі.
        int ready = display_lobby(sock, max_y, max_x, ip_buffer);
        
        if (ready) {
            // run_client тепер має приймати сокет, який вже підключений
            // Вам потрібно модифікувати run_client, щоб він приймав int sock
            printf("[CLIENT] Starting game...\n");
            return run_client_game(sock, skin_ship); 
        } else {
            close(sock);
            printf("[CLIENT] Disconnected from lobby.\n");
            return 0;
        }
    }
    return -1;
}

int display_lobby(int sock, int win_height, int win_width, const char *server_ip) {
    WINDOW *lobby_win;
    int ch;
    
    // Ініціалізація Ncurses для лобі
    initscr();
    cbreak();
    noecho();
    curs_set(0); 

    int start_x = (win_width - 60) / 2;
    int start_y = (win_height - 15) / 2;
    lobby_win = newwin(15, 60, start_y, start_x);
    box(lobby_win, 0, 0);
    keypad(lobby_win, TRUE);
    nodelay(lobby_win, TRUE);

    mvwprintw(lobby_win, 1, 2, "=== MULTIPLAYER LOBBY ===");
    mvwprintw(lobby_win, 3, 2, "STATUS: Connected to Server [%s]", server_ip);
    mvwprintw(lobby_win, 5, 2, "PLAYER 1 (Server): READY");
    mvwprintw(lobby_win, 6, 2, "PLAYER 2 (Client): READY");
    
    mvwprintw(lobby_win, 8, 2, "Press 'S' to Start the game");
    mvwprintw(lobby_win, 9, 2, "Press 'Q' to Disconnect");

    wrefresh(lobby_win);

    int ready_to_start = 0;
    while(1) {
        ch = wgetch(lobby_win);

        if (ch == 's' || ch == 'S') {
            ready_to_start = 1;
            break;
        }
        if (ch == 'q' || ch == 'Q') {
            ready_to_start = 0;
            break;
        }
        napms(100); 
    }

    delwin(lobby_win);
    endwin(); // Завершення ncurses для лобі

    return ready_to_start;
}

// Помістіть це у mechanics.c (або в окремий файл, якщо потрібно)
int display_server_lobby(int win_height, int win_width) {
    WINDOW *lobby_win;
    int ch;
    
    // Ініціалізація Ncurses для лобі
    initscr();
    cbreak();
    noecho();
    curs_set(0); 

    int start_x = (win_width - 60) / 2;
    int start_y = (win_height - 15) / 2;
    lobby_win = newwin(15, 60, start_y, start_x);
    box(lobby_win, 0, 0);
    keypad(lobby_win, TRUE);
    nodelay(lobby_win, FALSE); // БЛОКУЮЧИЙ РЕЖИМ

    mvwprintw(lobby_win, 1, 2, "=== MULTIPLAYER HOST LOBBY ===");
    mvwprintw(lobby_win, 3, 2, "STATUS: Client Connected.");
    mvwprintw(lobby_win, 5, 2, "PLAYER 1 (Host): READY");
    mvwprintw(lobby_win, 6, 2, "PLAYER 2 (Client): READY (Waiting for Host to start)");
    
    mvwprintw(lobby_win, 8, 2, "Press 'S' to Start the game");
    mvwprintw(lobby_win, 9, 2, "Press 'Q' to Disconnect");

    wrefresh(lobby_win);

    int ready_to_start = 0;
    while(1) {
        ch = wgetch(lobby_win);

        if (ch == 's' || ch == 'S') {
            ready_to_start = 1;
            break;
        }
        if (ch == 'q' || ch == 'Q') {
            ready_to_start = 0;
            break;
        }
    }

    delwin(lobby_win);
    endwin(); // Завершення ncurses для лобі

    return ready_to_start;
}

void input_move_player(Player *p, int ch, int win_width, Bullet bullets[]){ {
    switch (ch){
        case 'a':
        case KEY_LEFT:  
            move_left(p, 1);
            break;
        case 'd':
        case KEY_RIGHT: 
            move_right(p, win_width - 1);
            break;
        case ' ':
            if(p->bullets_left > 0){
                shoot(bullets, MAX_BULLETS, p);
                p->bullets_left--;
            }
            break;
    }
    }
}

void reset_game(Player *player, Bullet bullets[], Shatle shatles[], Asteroid asteroids[], int max_asteroids, int max_shatles, int *score, int *bullets_left, int *waiting_for_shatle_wave, int *shatle_wave_active, int *shatle_to_spawn, int *shatle_sequence_index, int *last_shatle_wave_score, int *shatle_wave_done, int *shatle_spawn_timer, int *spawn_timer, int *spawn_interval, int *game_running){
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
    *spawn_timer = 0;
    *spawn_interval = (rand() % 70) + 20;  // Скидаємо інтервал спавну

    *game_running = 1;
}

/*
void bullets_logics(WINDOW *win, Bullet bullets[], int max_bullets, int *bullets_left, int *bullets_recover_timer, int *score, Asteroid asteroids[], int max_asteroids, Shatle shatles[], int max_shatles) {
    move_bullets(bullets, MAX_BULLETS);
    draw_bullets(win, bullets, MAX_BULLETS);
    for(int b = 0; b < MAX_BULLETS; b++){
        if(!bullets[b].active) continue;
        for(int a = 0; a < MAX_ASTEROIDS; a++){
            if(bullet_hits_asteroid(&bullets[b], &asteroids[a])){
                *score += 5;
                bullets[b].active = 0;
            }
        }
        for(int s = 0; s < MAX_SHATLES; s++){
            int result = bullet_hits_shatle(&bullets[b], &shatles[s]);
            if(result > 1){
                *score += result;
                bullets[b].active = 0;
            }
        }
    }

    *bullets_recover_timer += 10;
    if(*bullets_recover_timer >= 2500){
        if(*bullets_left < MAX_BULLETS){
           (*bullets_left)++;
        }
        *bullets_recover_timer = 0;
    }
}
    */

void bullets_recovery_logics(Player *p) {
    p->bullets_recover_timer += 10;
    
    if (p->bullets_recover_timer >= 2500) {
        if (p->bullets_left < MAX_BULLETS) {
           (p->bullets_left)++;
        }
        p->bullets_recover_timer = 0;
    }
}

void bullets_logics_and_movement(WINDOW *win, Bullet bullets[], int max_bullets, int *score, Asteroid asteroids[], int max_asteroids, Shatle shatles[], int max_shatles) {
    
    move_bullets(bullets, MAX_BULLETS);
    draw_bullets(win, bullets, MAX_BULLETS);
    
    for(int b = 0; b < max_bullets; b++){ // Використовуйте max_bullets з аргументів
        if(!bullets[b].active) continue;

        for(int a = 0; a < max_asteroids; a++){
            if(bullet_hits_asteroid(&bullets[b], &asteroids[a])){
                *score += 5;
                bullets[b].active = 0;
                break; // Куля вже не активна, переходимо до наступної кулі
            }
        }
        
        if(!bullets[b].active) continue; 
        
        for(int s = 0; s < max_shatles; s++){
            // Припускаємо, що bullet_hits_shatle повертає очки > 1, якщо влучила
            int result = bullet_hits_shatle(&bullets[b], &shatles[s]); 
            if(result > 1){
                *score += result;
                bullets[b].active = 0;
                break; // Куля вже не активна, переходимо до наступної кулі
            }
        }
    }
}

void shatles_logics(WINDOW *win, Shatle shatles[], Asteroid asteroids[], int max_asteroids, int *score, int *last_shatle_wave_score, int *waiting_for_shatle_wave, int *shatle_wave_active, int *shatle_to_spawn, int *shatle_sequence_index, int win_height, int win_width, int *shatle_spawn_timer, int shatle_spawn_interval_ticks, Shatle *shatle_types[], int num_shatle_types, Asteroid *asteroid_types[], int num_types, int *spawn_timer, int *spawn_interval, int shatle_sequence[], int *shatle_wave_done) {
    if((*score / 100) > *last_shatle_wave_score && !(*shatle_wave_active)){
        *waiting_for_shatle_wave = 1;
        *last_shatle_wave_score = *score / 100;
    }

    if(*shatle_wave_active && *shatle_to_spawn > 0){
        (*shatle_spawn_timer)++;
        if(*shatle_spawn_timer >= shatle_spawn_interval_ticks){
            *shatle_spawn_timer = 0;
            int type_to_spawn = shatle_sequence[*shatle_sequence_index];
            if(shatles_spawn(shatles, MAX_SHATLES, shatle_types, num_shatle_types, type_to_spawn, win_height, win_width)){
                (*shatle_sequence_index)++;
                (*shatle_to_spawn)--;
            }
        }
    }

    for(int i = 0; i < MAX_SHATLES; i++){
        move_shatle(&shatles[i], win_width - 2);
        draw_shatle(win, &shatles[i]);
    }

    if(!(*waiting_for_shatle_wave) && !(*shatle_wave_active)){
        int max_active_asteroids = 15 + *score / 10;
        if(max_active_asteroids > MAX_ASTEROIDS) max_active_asteroids = MAX_ASTEROIDS;
        int active_count = 0;
        for(int i = 0; i < MAX_ASTEROIDS; i++){
            if(asteroids[i].active) active_count++;
        }
        if(active_count < max_active_asteroids){
            asteroids_spawn(asteroids, MAX_ASTEROIDS, asteroid_types, num_types, win_width, spawn_timer, spawn_interval, *shatle_wave_active);
        }
    }
    if(*waiting_for_shatle_wave){
        if(!any_asteroid_active(asteroids, MAX_ASTEROIDS)){
            *waiting_for_shatle_wave = 0;
            *last_shatle_wave_score = *score / 100;
            *shatle_wave_active = 1;
            *shatle_to_spawn = 3;
            shatle_sequence[0] = 1;
            shatle_sequence[1] = 0;
            shatle_sequence[2] = 0;
            shuffle_sequence(shatle_sequence, 3);
            *shatle_sequence_index = 0;
            *shatle_spawn_timer = 0;
        }
    }

    if(*shatle_wave_active && *shatle_to_spawn == 0){
        int any = 0;
        for(int i = 0; i < MAX_SHATLES; i++){
            if(shatles[i].active){ any = 1; break; }
        }
        if(!any){
            *shatle_wave_active = 0;
            *shatle_wave_done = 1;
            *spawn_timer = 0;
        }
    }
}

void asteroids_logics(WINDOW *win, Asteroid asteroids[], int max_asteroids, int *score, int *spawn_timer, int *spawn_interval, Asteroid *asteroid_types[], int num_types, int win_height, int win_width, Player *p1, Player *p2, int *game_running_flag) {
    for(int i = 0; i < MAX_ASTEROIDS; i++){
        if(asteroids[i].active){
            move_down(&asteroids[i], win_height - 1);
            draw_asteroid(win, &asteroids[i]);
        }
    }
    
    int collision_detected = 0;
    
    for(int i = 0; i < MAX_ASTEROIDS; i++){
        if(check_collision(&asteroids[i], p1)){
            collision_detected = 1;
            break;
        }
        if(p2 != NULL && p2->active) {
            if(check_collision(&asteroids[i], p2)){
                collision_detected = 1;
                break;
            }
        }
    }
    if(collision_detected){
        *game_running_flag = 0;
    }
}

void score_difficulty_logics(int *score, int *score_timer, Asteroid asteroids[], int max_asteroids, Shatle shatles[], int max_shatles) {
    (*score_timer)++;
    if(*score_timer >= 100){
        (*score)++;
        *score_timer = 0;
    }

    float difficulty_factor = 1.0 + (*score / 100);
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
    min_spawn_delay = 10 - (*score / 10);
    if(min_spawn_delay < 0.05) min_spawn_delay = 0.05;
    
    int max_active_asteroids = 20 + *score / 10;
    if(max_active_asteroids > MAX_ASTEROIDS) max_active_asteroids = MAX_ASTEROIDS;
}

int main_menu(WINDOW *win, int win_height, int win_width, int ch) {
    werase(win);
    box(win, 0, 0);
    mvwprintw(win, (win_height / 2) - 3, (win_width / 2) - 7, "Start Game (G)");
    mvwprintw(win, (win_height / 2) - 1, (win_width / 2) - 5, "Skins (S)");
    mvwprintw(win, (win_height / 2) + 1, (win_width / 2) - 7, "Host Server (H)");
    mvwprintw(win, (win_height / 2) + 3, (win_width / 2) - 6, "Join Game (J)");
    mvwprintw(win, (win_height / 2) + 5, (win_width / 2) - 4, "Quit (Q)");

    switch (ch){
        case 'g':
        case 'G':
            return ACTION_PLAY_SINGLE;
        case 's':
        case 'S':
            return ACTION_SKINS;
        case 'h':
        case 'H':
            return ACTION_HOST;
        case 'j':
        case 'J':
            return ACTION_JOIN;
        case 'q':
        case 'Q':
            return ACTION_QUIT;
        default:
            return ACTION_MENU;
    }
}