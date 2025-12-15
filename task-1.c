#include <superkarel.h>

bool cheker(){
    if(front_is_clear()){
        step();
    }else{
        return false;
    }
    if(front_is_clear()){
        turn_left();
        turn_left();
        step();
        turn_left();
        turn_left();
        return true;
    }else{
        return false;
    }
}

bool chek_beepers_in_line(){
    put_beeper();
    do{
        step();
        if(beepers_present()){
            turn_left();
            turn_left();
            do{
                step();
            }while (no_beepers_present());
            turn_left();
            turn_left();
            pick_beeper();
            return true;
        }
    }while (front_is_clear());
    turn_left();
    turn_left();
    do{
        step();
    }while (no_beepers_present());
    pick_beeper();
    turn_left();
    turn_left();
    return false;
}

void find_centr2(){
    if(cheker()){
        step();
        step();
        if(cheker()){
            find_centr2();
        }
        while(not_facing_east()){
            turn_left();
        }
        step();
    }
}

void zerkalo(){
    if(front_is_clear() && no_beepers_in_bag()){
        step();
        if(beepers_present()){
            pick_beeper();
        }
        zerkalo();
        while(not_facing_east()){
            turn_left();
        }
        step();
        step();
    }
}

bool chek_2_beepers(){
    if(beepers_present()){
        pick_beeper();
        if(beepers_present()){
            put_beeper();
            return true;
        }else{
            put_beeper();
            return false;
        }
    }else{
        return false;
    }
}

int main(){
    turn_on("task-1.kw");
    set_step_delay(50);
    while (not_facing_south())
    {
        turn_left();
    }
    while (front_is_clear())
    {
        step();
    }
    while (not_facing_east())
    {
        turn_left();
    }
    while (front_is_clear())
    {
        step();
    }
    turn_left();
    turn_left();
    

    find_centr2();
    if(chek_beepers_in_line()){
        while(chek_beepers_in_line()){
                while(beepers_in_bag()){
                    put_beeper();
                }
                zerkalo();
                put_beeper();
                while (front_is_clear())
                {
                    step();
                }
                
                turn_left();
                turn_left();
                while(!chek_2_beepers()){
                    step();
                }
                while (beepers_present())
                {
                    pick_beeper();
                }
            }
            turn_left();
            turn_left();
    }
    while(true){
        turn_left();
        if(front_is_blocked()){
            break;
        }
        step();
        turn_left();
        while(chek_beepers_in_line()){
            while(beepers_in_bag()){
                put_beeper();
            }
            zerkalo();
            put_beeper();
            while (front_is_clear())
            {
                step();
            }
            
            turn_left();
            turn_left();
            while(!chek_2_beepers()){
                step();
            }
            while (beepers_present())
            {
                pick_beeper();
            }
        }
        turn_left();
        turn_left();

    }
    turn_left();
    turn_left();
    while (front_is_clear())
    {
        step();
    }
    turn_left();
    while (front_is_clear())
    {
        step();
    }
    
    turn_off();
    return 0;
}