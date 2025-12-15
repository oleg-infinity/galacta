//skins.c

#include <stdlib.h>
#include "skins.h"

// skins menu

const char *ship_skin1[] = {
    ". .",
    "|^|",
    "/0\\",
    NULL
};

const char *ship_skin2[] = {
    ". .",
    "|A|",
    "/M\\",
    NULL
};

const char *ship_skin3[] = {
    " !^! ",
    "({O})",
    "<vAv>",
    NULL
};

const char *ship_skin4[] = {
    " . . ",
    "-|^|-",
    "/0 0\\",
    NULL
};

const char *ship_skin5[] = {
    "  .  ",
    " / \\ ",
    "/=O=\\",
    NULL
};

const char *ship_skin6[] = {
    " . . ",
    "/|^|\\",
    "#0_0&",
    NULL
};

const char *ship_skin7[] = {
    "  .  ",
    " /|\\ ",
    "/0+0\\",
    NULL
};

const char *ship_skin8[] = {
    ". | .",
    "\\/|\\/ ",
    "/^+^\\",
    NULL
};

const char *ship_skin9[] = {
    " .^. ",
    "/|O|\\",
    "/___\\",
    NULL
};

const char *ship_skin10[] = {
    "  .  ",
    " /U\\ ",
    "/K S\\",
    NULL
};


const char **available_skins[] = {ship_skin1, ship_skin2, ship_skin3, ship_skin4, ship_skin5, ship_skin6, ship_skin7, ship_skin8, ship_skin9, ship_skin10};
int num_available_skins = sizeof(available_skins) / sizeof(available_skins[0]);

void draw_skin(WINDOW *win, const char **skin, int skin_width, int skin_height, int start_y, int start_x){
    for(int i = 0; i < skin_height; i++){
        for(int j = 0; j < skin_width; j++){
            char c = skin[i][j];
            if(c != ' '){
                mvwprintw(win, start_y + i, start_x + j, "%c", c);
            }
        }    
    }
}