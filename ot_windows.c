
#include "ot_windows.h"

/*Data for FSM*/
window_t windows[NUM_WINDOWS];

/*FSM STATUS*/
enum
{
    IDLE,
    BLOCKED_BOTH,
    BLOCKED_UP,
    BLOCKED_DOWN,
    START_DOWN,
    START_UP,
    AUTO_DOWN,
    AUTO_UP,
    MAN_DOWN,
    MAN_UP
    };


void window_fsm_fire(window_t* current_window){

    current_window->current_state=current_window->next_state; // Updates state

    switch (current_window->current_state) {
        case IDLE:
        {
            if (((current_window->flags).up_sw && (current_window->flags).down_sw) || (!(current_window->flags).up_sw && !(current_window->flags).down_sw)) {
                clear_safety_flags(&(current_window->flags));
                clear_input_flags(&(current_window->flags));
                turn_off(&(current_window->flags));
                current_window->next_state = IDLE;
            }
            if ((current_window->flags).up_sw && !(current_window->flags).down_sw) { //Up botton pressed
                clear_input_flags(&(current_window->flags));
                set_output(&(current_window->flags), UP);
                set_comparators(&(current_window->flags), UP);
                set_safety_timer(current_window, TIMEOUT_UP);
                set_OT_timer(current_window, OT_TIMER_COUNT);
                current_window->next_state = START_UP;
            }
            if (!(current_window->flags).up_sw && (current_window->flags).down_sw) { //Down botton pressed
                clear_input_flags(&(current_window->flags));
                set_output(&(current_window->flags),DOWN);
                set_comparators(&(current_window->flags),DOWN);
                set_safety_timer(current_window,TIMEOUT_DOWN);
                set_OT_timer(current_window, OT_TIMER_COUNT);
                current_window->next_state = START_DOWN;
            }
            break;
        }
        case START_DOWN:
        {
            if (!(current_window->flags).current_sense && (current_window->flags).current_sense_timer_enable){
                clear_safety_flags(&(current_window->flags));
                turn_off_CS_timer(&(current_window->flags));                
                current_window->next_state=START_DOWN;
                break;
            }       
            if ((current_window->flags).current_sense && !(current_window->flags).current_sense_timer_enable) {
                clear_CS_flags(&(current_window->flags));
                set_CS_timer(current_window, CS_TIMER_COUNT);
                current_window->next_state = START_DOWN;
                break; 
            }
            if ((current_window->flags).timeout || 
                ((current_window->flags).current_sense && (current_window->flags).current_sense_timer_rollover )){ // If safety timeout stops window
                clear_safety_flags(&(current_window->flags));
                if ((current_window->flags).up_sw && !(current_window->flags).down_sw) {
                    clear_input_flags(&(current_window->flags));
                    turn_off(&(current_window->flags));
                    current_window->next_state = BLOCKED_UP;
                    break;
                }
                if (!(current_window->flags).up_sw && (current_window->flags).down_sw) {
                    clear_input_flags(&(current_window->flags));
                    turn_off(&(current_window->flags));
                    current_window->next_state = BLOCKED_DOWN;
                    break;
                }
                if (!(current_window->flags).up_sw && !(current_window->flags).down_sw) {
                    clear_input_flags(&(current_window->flags));
                    turn_off(&(current_window->flags));
                    current_window->next_state = IDLE;
                    break;
                }
                if ((current_window->flags).up_sw && (current_window->flags).down_sw) {
                    clear_input_flags(&(current_window->flags));
                    turn_off(&(current_window->flags));
                    current_window->next_state = BLOCKED_BOTH;
                    break;
                }
            }
            if (!(current_window->flags).up_sw) {
                if (!(current_window->flags).ot_timer_rollover && !(current_window->flags).down_sw) { //If time hasn't passed and button is released goto OTD
                    clear_input_flags(&(current_window->flags));
                    set_output(&(current_window->flags), DOWN);
                    turn_off_OT_timer(&(current_window->flags));
                    current_window->next_state = AUTO_DOWN;
                    break;
                }
                if ((current_window->flags).ot_timer_rollover && (current_window->flags).down_sw) { // If time has passed and button is still pressed goto MAN_DOWN
                    clear_input_flags(&(current_window->flags));
                    set_output(&(current_window->flags), DOWN);
                    turn_off_OT_timer(&(current_window->flags));
                    current_window->next_state = MAN_DOWN;
                    break;
                }
                if (!(current_window->flags).ot_timer_rollover && (current_window->flags).down_sw) { //If time hasn't passed and button is still pressed keep polling on START_DOWN
                    clear_input_flags(&(current_window->flags));
                    set_output(&(current_window->flags), DOWN);
                    current_window->next_state = START_DOWN;
                    break;
                }
                if ((current_window->flags).ot_timer_rollover && !(current_window->flags).down_sw) { //If button is released and time has passed goto IDLE state
                    clear_input_flags(&(current_window->flags));
                    turn_off(&(current_window->flags));
                    current_window->next_state = IDLE;
                    break;
                }
            } else {
                if ((current_window->flags).down_sw) {
                    clear_input_flags(&(current_window->flags));
                    turn_off(&(current_window->flags));
                    current_window->next_state = BLOCKED_BOTH; // If both pressed go to safety state
                    break;
                } else {
                    clear_input_flags(&(current_window->flags));
                    turn_off(&(current_window->flags)); // Allows changing rolling sense
                    current_window->next_state = IDLE;
                    break;
                }
            }
        }
             
        case START_UP:
        {
            if (!(current_window->flags).current_sense && (current_window->flags).current_sense_timer_enable) {
                clear_safety_flags(&(current_window->flags));
                turn_off_CS_timer(&(current_window->flags));
                current_window->next_state = START_UP;
                break;
            }
            if ((current_window->flags).current_sense && !(current_window->flags).current_sense_timer_enable) {
                clear_CS_flags(&(current_window->flags));
                set_CS_timer(current_window, CS_TIMER_COUNT);
                current_window->next_state = START_UP;
                break;
            }
            
            if ((current_window->flags).timeout || 
                ((current_window->flags).current_sense && (current_window->flags).current_sense_timer_rollover))  { // If a safety event has been trigged goto safety states
                clear_safety_flags(&(current_window->flags));
                if ((current_window->flags).up_sw && !(current_window->flags).down_sw) {
                    clear_input_flags(&(current_window->flags));
                    turn_off(&(current_window->flags));
                    current_window->next_state = BLOCKED_UP;
                    break;
                }
                if (!(current_window->flags).up_sw && (current_window->flags).down_sw) {
                    clear_input_flags(&(current_window->flags));
                    turn_off(&(current_window->flags));
                    current_window->next_state = BLOCKED_DOWN;
                    break;
                }
                if (!(current_window->flags).up_sw && !(current_window->flags).down_sw) {
                    clear_input_flags(&(current_window->flags));
                    turn_off(&(current_window->flags));
                    current_window->next_state = IDLE;
                    break;
                }
                if ((current_window->flags).up_sw && (current_window->flags).down_sw) {
                    clear_input_flags(&(current_window->flags));
                    turn_off(&(current_window->flags));
                    current_window->next_state = BLOCKED_BOTH;
                    break;
                }
            }
            if (!(current_window->flags).down_sw) {
                if (!(current_window->flags).ot_timer_rollover && !(current_window->flags).up_sw) { //If time hasn't passed and button is released goto OTD
                    clear_input_flags(&(current_window->flags));
                    set_output(&(current_window->flags), UP);
                    turn_off_OT_timer(&(current_window->flags));
                    current_window->next_state = AUTO_UP;
                    break;
                }
                if ((current_window->flags).ot_timer_rollover && (current_window->flags).up_sw) { // If time has passed and button is still pressed goto MAN_UP
                    clear_input_flags(&(current_window->flags));
                    set_output(&(current_window->flags), UP);
                    turn_off_OT_timer(&(current_window->flags));
                    current_window->next_state = MAN_UP;
                    break;
                }
                if (!(current_window->flags).ot_timer_rollover && (current_window->flags).up_sw) { //If time hasn't passed and button is still pressed keep polling on START_UP
                    clear_input_flags(&(current_window->flags));
                    set_output(&(current_window->flags), UP);
                    current_window->next_state = START_UP;
                    break;
                }
                if ((current_window->flags).ot_timer_rollover && !(current_window->flags).up_sw) { //If button is released and time has passed goto IDLE state
                    clear_input_flags(&(current_window->flags));
                    turn_off(&(current_window->flags));
                    current_window->next_state = IDLE;
                    break;
                }
            } else {
                if ((current_window->flags).up_sw) {
                    clear_input_flags(&(current_window->flags));
                    turn_off(&(current_window->flags)); // If both pressed go to safety state
                    current_window->next_state = BLOCKED_BOTH;
                    break;
                } else {
                    clear_input_flags(&(current_window->flags));
                    turn_off(&(current_window->flags)); // Allows changing rolling sense
                    current_window->next_state = IDLE;
                    break;
                }
            }
        }

        case AUTO_DOWN:
        {
            if (!(current_window->flags).current_sense && (current_window->flags).current_sense_timer_enable) {
                clear_safety_flags(&(current_window->flags));
                turn_off_CS_timer(&(current_window->flags));
                current_window->next_state = AUTO_DOWN;
                break;
            }
            if ((current_window->flags).current_sense && !(current_window->flags).current_sense_timer_enable) {
                clear_CS_flags(&(current_window->flags));
                set_CS_timer(current_window, CS_TIMER_COUNT);
                current_window->next_state = AUTO_DOWN;
                break;
            }
            if (!(current_window->flags).up_sw && !(current_window->flags).down_sw) {
                if ((current_window->flags).timeout ||                   
                        ((current_window->flags).current_sense && (current_window->flags).current_sense_timer_rollover)) { // If safety timeout stops window                    
                    clear_safety_flags(&(current_window->flags));
                    clear_input_flags(&(current_window->flags));
                    turn_off(&(current_window->flags)); // Rolling has ended
                    current_window->next_state = IDLE;
                    break;
                } else { // Keep polling
                    clear_input_flags(&(current_window->flags));
                    set_output(&(current_window->flags), DOWN);
                    current_window->next_state = AUTO_DOWN;
                    break;
                }
            }
            if ((current_window->flags).up_sw && !(current_window->flags).down_sw) { // Up is pressed to stop rolling
                clear_input_flags(&(current_window->flags));
                turn_off(&(current_window->flags));
                current_window->next_state = BLOCKED_UP;
                break;
            }
            if (!(current_window->flags).up_sw && (current_window->flags).down_sw) { // Down is pressed to stop rolling
                clear_input_flags(&(current_window->flags));
                turn_off(&(current_window->flags));
                current_window->next_state = BLOCKED_DOWN;
                break;
            }
            if ((current_window->flags).up_sw && (current_window->flags).down_sw) {
                clear_input_flags(&(current_window->flags));
                turn_off(&(current_window->flags));
                current_window->next_state = BLOCKED_BOTH;
                break;
            }
        }

        case AUTO_UP:
        {
            if (!(current_window->flags).current_sense && (current_window->flags).current_sense_timer_enable) {
                clear_safety_flags(&(current_window->flags));
                turn_off_CS_timer(&(current_window->flags));
                current_window->next_state = AUTO_UP;
                break;
            }
            if ((current_window->flags).current_sense && !(current_window->flags).current_sense_timer_enable) {
                clear_CS_flags(&(current_window->flags));
                set_CS_timer(current_window, CS_TIMER_COUNT);
                current_window->next_state = AUTO_UP;
                break;
            }
            if (!(current_window->flags).up_sw && !(current_window->flags).down_sw) {
                if ((current_window->flags).timeout ||
                        ((current_window->flags).current_sense && (current_window->flags).current_sense_timer_rollover)) {
                    clear_safety_flags(&(current_window->flags));
                    clear_input_flags(&(current_window->flags));
                    turn_off(&(current_window->flags)); // Rolling has ended
                    current_window->next_state = IDLE;
                    break;
                } else { // Keep polling
                    clear_input_flags(&(current_window->flags));
                    set_output(&(current_window->flags), UP);
                    current_window->next_state = AUTO_UP;
                    break;
                }
            }
            if ((current_window->flags).up_sw && !(current_window->flags).down_sw) { // Up is pressed to stop rolling
                clear_input_flags(&(current_window->flags));
                turn_off(&(current_window->flags));
                current_window->next_state = BLOCKED_UP;
                break;
            }
            if (!(current_window->flags).up_sw && (current_window->flags).down_sw) { // Down is pressed to stop rolling
                clear_input_flags(&(current_window->flags));
                turn_off(&(current_window->flags));
                current_window->next_state = BLOCKED_DOWN;
                break;
            }
            if ((current_window->flags).up_sw && (current_window->flags).down_sw) {
                clear_input_flags(&(current_window->flags));
                turn_off(&(current_window->flags));
                current_window->next_state = BLOCKED_BOTH;
                break;
            }
        }

        case MAN_DOWN:
        {
            if (!(current_window->flags).current_sense && (current_window->flags).current_sense_timer_enable) {
                clear_safety_flags(&(current_window->flags));
                turn_off_CS_timer(&(current_window->flags));
                current_window->next_state = MAN_DOWN;
                break;
            }
            if ((current_window->flags).current_sense && !(current_window->flags).current_sense_timer_enable) {
                clear_CS_flags(&(current_window->flags));
                set_CS_timer(current_window, CS_TIMER_COUNT);
                current_window->next_state = MAN_DOWN;
                break;
            }
            if ((current_window->flags).timeout || 
                    ((current_window->flags).current_sense && (current_window->flags).current_sense_timer_rollover)) {
                clear_safety_flags(&(current_window->flags));
                if ((current_window->flags).up_sw && !(current_window->flags).down_sw) {
                    clear_input_flags(&(current_window->flags));
                    turn_off(&(current_window->flags));
                    current_window->next_state = BLOCKED_UP;
                    break;
                }
                if (!(current_window->flags).up_sw && (current_window->flags).down_sw) {
                    clear_input_flags(&(current_window->flags));
                    turn_off(&(current_window->flags));
                    current_window->next_state = BLOCKED_DOWN;
                    break;
                }
                if (!(current_window->flags).up_sw && !(current_window->flags).down_sw) {
                    clear_input_flags(&(current_window->flags));
                    turn_off(&(current_window->flags));
                    current_window->next_state = IDLE;
                    break;
                }
                if ((current_window->flags).up_sw && (current_window->flags).down_sw) {
                    clear_input_flags(&(current_window->flags));
                    turn_off(&(current_window->flags));
                    current_window->next_state = BLOCKED_BOTH;
                    break;
                }
            }
            if ((current_window->flags).up_sw && !(current_window->flags).down_sw) {
                clear_input_flags(&(current_window->flags));
                turn_off(&(current_window->flags));
                current_window->next_state = IDLE;
                break;
            }
            if (!(current_window->flags).up_sw && (current_window->flags).down_sw) {
                clear_input_flags(&(current_window->flags));
                current_window->next_state = MAN_DOWN;
                break;
            }
            if (!(current_window->flags).up_sw && !(current_window->flags).down_sw) {
                clear_input_flags(&(current_window->flags));
                turn_off(&(current_window->flags));
                current_window->next_state = IDLE;
                break;
            }
            if ((current_window->flags).up_sw && (current_window->flags).down_sw) {
                clear_input_flags(&(current_window->flags));
                turn_off(&(current_window->flags));
                current_window->next_state = BLOCKED_BOTH;
                break;
            }
        }

        case MAN_UP:
        {
            if (!(current_window->flags).current_sense && (current_window->flags).current_sense_timer_enable) {
                clear_safety_flags(&(current_window->flags));
                turn_off_CS_timer(&(current_window->flags));
                current_window->next_state = MAN_UP;
                break;
            }
            if ((current_window->flags).current_sense && !(current_window->flags).current_sense_timer_enable) {
                clear_CS_flags(&(current_window->flags));
                set_CS_timer(current_window, CS_TIMER_COUNT);
                current_window->next_state = MAN_UP;
                break;
            }
            if ((current_window->flags).timeout ||
                    ((current_window->flags).current_sense && (current_window->flags).current_sense_timer_rollover)) {
                clear_safety_flags(&(current_window->flags));
                if ((current_window->flags).up_sw && !(current_window->flags).down_sw) {
                    clear_input_flags(&(current_window->flags));
                    turn_off(&(current_window->flags));
                    current_window->next_state = BLOCKED_UP;
                    break;
                }
                if (!(current_window->flags).up_sw && (current_window->flags).down_sw) {
                    clear_input_flags(&(current_window->flags));
                    turn_off(&(current_window->flags));
                    current_window->next_state = BLOCKED_DOWN;
                    break;
                }
                if (!(current_window->flags).up_sw && !(current_window->flags).down_sw) {
                    clear_input_flags(&(current_window->flags));
                    turn_off(&(current_window->flags));
                    current_window->next_state = IDLE;
                    break;
                }
                if ((current_window->flags).up_sw && (current_window->flags).down_sw) {
                    clear_input_flags(&(current_window->flags));
                    turn_off(&(current_window->flags));
                    current_window->next_state = BLOCKED_BOTH;
                    break;
                }
            }
            if ((current_window->flags).up_sw && !(current_window->flags).down_sw) {
                clear_input_flags(&(current_window->flags));
                current_window->next_state = MAN_UP;
                break;
            }
            if (!(current_window->flags).up_sw && (current_window->flags).down_sw) {
                clear_input_flags(&(current_window->flags));
                turn_off(&(current_window->flags));
                current_window->next_state = IDLE;
                break;
            }
            if (!(current_window->flags).up_sw && !(current_window->flags).down_sw) {
                clear_input_flags(&(current_window->flags));
                turn_off(&(current_window->flags));
                current_window->next_state = IDLE;
                break;
            }
            if ((current_window->flags).up_sw && (current_window->flags).down_sw) {
                clear_input_flags(&(current_window->flags));
                turn_off(&(current_window->flags));
                current_window->next_state = BLOCKED_BOTH;
                break;
            }
        }

        case BLOCKED_DOWN:{
            
            turn_off(&(current_window->flags));
            if ((current_window->flags).down_sw) {
                current_window->next_state = BLOCKED_DOWN;
            }
            else {
                current_window->next_state = IDLE;
            }
            clear_input_flags(&(current_window->flags));
            break;
        }

        case BLOCKED_UP:{
            turn_off(&(current_window->flags));
            if ((current_window->flags).up_sw) {
                current_window->next_state = BLOCKED_UP;
            }
            else {
                current_window->next_state = IDLE;
            }
            clear_input_flags(&(current_window->flags));
            break;
        }

        case BLOCKED_BOTH:{
            turn_off(&(current_window->flags));
            if ((current_window->flags).up_sw || (current_window->flags).down_sw) {
                current_window->next_state = BLOCKED_BOTH;
            }
            else {
                current_window->next_state = IDLE;
            }
            clear_input_flags(&(current_window->flags));
            break;
        }
    }
}

inline void windows_fsm_fire_all(){
    unsigned char i;
    for(i=0;i<NUM_WINDOWS;i++){
        window_fsm_fire(&windows[i]);
    }
}

void windows_init(void) {
    unsigned char i;
    for (i = 0; i < NUM_WINDOWS; i++) {
        windows[i].current_state = IDLE;
        windows[i].next_state = IDLE;
        windows[i].ot_timer_counter = 0;
        windows[i].timeout_timer_counter = 0;
        windows[i].current_sense_timer_counter = 0;
        windows[i].flags.output_up = OFF;
        windows[i].flags.output_down = OFF;
        windows[i].flags.up_sw = OFF;
        windows[i].flags.down_sw = OFF;
        windows[i].flags.current_sense = OFF;
        windows[i].flags.current_sense_enable = OFF;
        windows[i].flags.current_sense_timer_rollover=OFF;
        windows[i].flags.current_sense_timer_enable = OFF;
        windows[i].flags.ot_timer_rollover = OFF;
        windows[i].flags.ot_timer_enable = OFF;
        windows[i].flags.timeout = OFF;
        windows[i].flags.timeout_enable = OFF;
    }
}

window_t get_window(unsigned char window){    
        return windows[window];
}

window_t* get_window_pointer (unsigned char window) {
        return (windows+window);
}

unsigned char set_window_id(unsigned char window_index, unsigned char id) {
    if (window_index < NUM_WINDOWS) {
        windows[window_index].id = id;
        return 1;
    }
    return 0;
}

inline void set_output(volatile FLAGS* flags, unsigned char dir) {
    switch (dir) {
        case UP:
        {
            flags->output_down = OFF;
            flags->output_up = ON;
            return;
        }
        case DOWN:
        {
            flags->output_up = OFF;
            flags->output_down = ON;
            return;
        }
        case OFF:
        default:
        {
            flags->output_up = OFF;
            flags->output_down = OFF;
            return;
        }
    }

}

inline void set_comparators(volatile FLAGS* flags, unsigned char dir) {
    flags->current_sense_enable = ON;
    return;
}

inline void set_safety_timer(window_t* current_window, int timeout) {
    current_window->timeout_timer_max_count = timeout;
    current_window->timeout_timer_counter = 0;
    current_window->flags.timeout = 0;
    current_window->flags.timeout_enable = ON;
    return;
}

inline void set_OT_timer(window_t* current_window, int time) {
    current_window->ot_timer_max_count = time;
    current_window->ot_timer_counter = 0;
    current_window->flags.ot_timer_rollover = 0;
    current_window->flags.ot_timer_enable = ON;
    return;
}

inline void set_CS_timer(window_t* current_window, int time) {
    current_window->current_sense_timer_max_count = time;
    current_window->current_sense_timer_counter = 0;
    current_window->flags.current_sense_timer_rollover = 0;
    current_window->flags.current_sense_timer_enable = ON;
    return;
}

/*Cleans safety events flags*/
inline void clear_safety_flags(volatile FLAGS* flags){
    flags->ot_timer_rollover=0;
    flags->current_sense=0;
    flags->current_sense_timer_rollover=0;
    flags->timeout=0;
    return;
}

inline void clear_input_flags(volatile FLAGS* flags){
    flags->down_sw=0;
    flags->up_sw=0;
    return;
}

inline void clear_CS_flags(volatile FLAGS* flags){
    flags->current_sense=0;
    flags->current_sense_timer_rollover=0;
    return;
}

inline void turn_off(volatile FLAGS* flags){
    flags->output_up=OFF;
    flags->output_down=OFF;
    flags->current_sense_enable=OFF;
    flags->ot_timer_enable=OFF;
    flags->current_sense_timer_enable=OFF;
    flags->timeout_enable=OFF;
    return;
}

inline void turn_off_OT_timer(volatile FLAGS* flags){
    flags->ot_timer_enable=OFF;
    return;
}

inline void turn_off_CS_timer (volatile FLAGS* flags){
    flags->current_sense_timer_enable=OFF;
    return;
}

void timer_init(void){
    TIMER_PR=TIMER_PR_LOAD;
    TIMER_TMR=0;
    TIMER_IF=0;
    TIMER_TCON=TIMER_TCON_LOAD;
    TIMER_IE=ON;
    PEIE=ON;
}

void port_init(void){
#ifdef TEST_4550
    ADCON1bits.PCFG=0b1111; //No analog inputs
#endif
    
    WINDOW1_SW_UP1_TRIS = ON;
    WINDOW1_SW_UP2_TRIS = ON;
    WINDOW1_SW_DOWN1_TRIS = ON;
    WINDOW1_SW_DOWN2_TRIS = ON;
    WINDOW1_ROLLUP_TRIS = OFF;
    WINDOW1_ROLLDOWN_TRIS = OFF;
    WINDOW1_CURRENT_SENSE_TRIS = ON;
    
    WINDOW2_SW_UP1_TRIS = ON;
    WINDOW2_SW_UP2_TRIS = ON;
    WINDOW2_SW_DOWN1_TRIS = ON;
    WINDOW2_SW_DOWN2_TRIS = ON;
    WINDOW2_ROLLUP_TRIS = OFF;
    WINDOW2_ROLLDOWN_TRIS = OFF;
    WINDOW2_CURRENT_SENSE_TRIS = ON;
    
    WINDOW1_ROLLUP=OFF;
    WINDOW1_ROLLDOWN = OFF;
    
    WINDOW2_ROLLUP=OFF;
    WINDOW2_ROLLDOWN=OFF;
}

void read_port(void){
    windows[0].flags.up_sw=(!WINDOW1_SW_UP1 || !WINDOW1_SW_UP2);
    windows[0].flags.down_sw=(!WINDOW1_SW_DOWN1 || !WINDOW1_SW_DOWN2);
    if (windows[0].flags.current_sense_enable){
        windows[0].flags.current_sense=WINDOW1_CURRENT_SENSE;
    }
    
    windows[1].flags.up_sw = (!WINDOW2_SW_UP1 || !WINDOW2_SW_UP2);
    windows[1].flags.down_sw = (!WINDOW2_SW_DOWN1 || !WINDOW2_SW_DOWN2);
    if (windows[1].flags.current_sense_enable) {
        windows[1].flags.current_sense = WINDOW2_CURRENT_SENSE;
    }
    
}

void set_port(void){
    WINDOW1_ROLLUP=windows[0].flags.output_up;
    WINDOW1_ROLLDOWN=windows[0].flags.output_down;
    
    WINDOW2_ROLLUP=windows[1].flags.output_up;
    WINDOW2_ROLLDOWN=windows[1].flags.output_down;    
}

void set_timer_flags(void){
    unsigned char i;
    for (i = 0; i < NUM_WINDOWS; i++) {
        if (windows[i].timeout_timer_counter >= windows[i].timeout_timer_max_count) {
            windows[i].flags.timeout = ON;
        }
        if (windows[i].ot_timer_counter >= windows[i].ot_timer_max_count) {
            windows[i].flags.ot_timer_rollover = ON;
        }
        if (windows[i].current_sense_timer_counter>=windows[i].current_sense_timer_max_count){
            windows[i].flags.current_sense_timer_rollover = ON;
        }
    }
}

void timer_interrupt(void){
    if (TIMER_IF && TIMER_IE){
        TIMER_IF=0;
        unsigned char i;
        for (i=0; i<NUM_WINDOWS; i++){
            if(windows[i].flags.timeout_enable){
                windows[i].timeout_timer_counter++;
                /*
                if(windows[i].timeout_timer_counter>=windows[i].timeout_timer_max_count){
                    windows[i].flags.timeout=ON;
                }
                 */
            }
            if(windows[i].flags.ot_timer_enable ){
                windows[i].ot_timer_counter++;
                /*
                if (windows[i].ot_timer_counter >= windows[i].ot_timer_max_count) {
                    windows[i].flags.ot_timer_rollover = ON;
                }
                 */

            }
            if (windows[i].flags.current_sense_timer_enable){
                windows[i].current_sense_timer_counter++;
            }

        }
    }
}











