#include <stdio.h>
#include <stdlib.h>

int load_score(){
    FILE *file = fopen("score.json", "r");
    if(!file) return 0;

    int score = 0;;
    if(fscanf(file, "{\"score\":%d}", &score) != 1){
        fclose(file);
        return 0;
    }
    fclose(file);
    return score;
}

void save_score(int score){
    FILE *file = fopen("score.json", "w");
    if(!file) return;

    fprintf(file, "{\"score\":%d}", score);
    fclose(file);
}