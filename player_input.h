//player_input.h

#ifndef PLAYER_INPUT_H
#define PLAYER_INPUT_H

#include "player.h"

// функція руху гравця за символом клавіатури
void input_move_player(Player *p, int ch, int win_width, Bullet bullets[]);

#endif
