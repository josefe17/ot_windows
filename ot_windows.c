
#include "ot_windows.h"

/*Data for FSM*/
window_t windows[NUM_WINDOWS];

void window_fsm_fire(window_t* current_window)
{
    current_window -> current_state = current_window -> next_state; // Updates state
	
    switch (current_window -> current_state)
	{
		default:
		case WINDOW_FSM_STATE.IDLE:
        {
            if (check_all_released(current_window) && !check_central_close(current_window)) // Idle
			{                
                clear_window_fsm_input_flags(&(current_window -> flags));
				turn_off_window_OT_timer(&(current_window -> flags));             
                current_window -> next_state = WINDOW_FSM_STATE.IDLE;
				break;
            }
			if (check_all_released(current_window) && check_central_close(current_window)) // Central close
			{
				clear_window_fsm_input_flags(&(current_window->flags));
				set_authorization_request_flags(&(current_window->flags), ON);
				set_output(current_window, OUTPUT_REQUEST.UP_PRESSED);
				current_window -> next_state = WINDOW_FSM_STATE.CENTRAL_CLOSE;
				break;				
			}
            if (check_up_and_no_down(current_window)) //Up botton pressed
			{
                clear_window_fsm_input_flags(&(current_window->flags));
				set_window_OT_timer(current_window, OT_TIMER_COUNT);
                set_output(current_window, OUTPUT_REQUEST.UP_PRESSED);
                current_window -> next_state = WINDOW_FSM_STATE.MAN_UP;
				break;
            }
            if (check_down_and_no_up(current_window)) //Down botton pressed
			{
                clear_window_fsm_input_flags(&(current_window->flags));
				set_window_OT_timer(current_window, OT_TIMER_COUNT);
                set_output(current_window, OUTPUT_REQUEST.DOWN_PRESSED);           
                current_window -> next_state = WINDOW_FSM_STATE.MAN_DOWN;
				break;
            }
			clear_window_fsm_input_flags(&(current_window->flags));
			turn_off_window_OT_timer(&(current_window -> flags));			
			set_output(current_window, OUTPUT_REQUEST.NONE);
            current_window -> next_state = WINDOW_FSM_STATE.BLOCKED;
			break;			
        }		
		case WINDOW_FSM_STATE.MAN_DOWN:
		{
			if (check_down_and_no_up(current_window))
			{
				clear_window_fsm_input_flags(&(current_window->flags));
				current_window->next_state = WINDOW_FSM_STATE.MAN_DOWN;
				break;
			}
			if (check_all_released(current_window))
			{
				if (check_ot_time_rollover(current_window)) // Manual mode
				{
					clear_window_fsm_input_flags(&(current_window -> flags));
					turn_off_window_OT_timer(&(current_window -> flags));
					set_output(current_window, OUTPUT_REQUEST.DOWN_RELEASED_STOP);
					current_window -> next_state = WINDOW_FSM_STATE.IDLE;
					break;
				}
				else // Auto mode
				{
					clear_window_fsm_input_flags(&(current_window -> flags));
					turn_off_window_OT_timer(&(current_window -> flags));
					set_output(current_window, OUTPUT_REQUEST.DOWN_RELEASED_AUTO);
					current_window -> next_state = WINDOW_FSM_STATE.IDLE;
					break;
				}
			}
			if (check_up_and_down(current_window)) //Auto mode double-click button
			{
				clear_window_fsm_input_flags(&(current_window -> flags));
				turn_off_window_OT_timer(&(current_window -> flags));
				current_window->next_state = WINDOW_FSM_STATE.AUTO_DOWN;
				break;
			}
			clear_window_fsm_input_flags(&(current_window -> flags));
			turn_off_window_OT_timer(&(current_window -> flags));
			set_output(current_window, OUTPUT_REQUEST.DOWN_RELEASED_STOP);
			current_window -> next_state = WINDOW_FSM_STATE.BLOCKED;
			break;
		}		
        case WINDOW_FSM_STATE.MAN_UP:
        {
            if (check_up_and_no_down(current_window))
			{
                clear_window_fsm_input_flags(&(current_window->flags));
                current_window->next_state = WINDOW_FSM_STATE.MAN_UP;
                break;
            }
            if (check_all_released(current_window))
			{
				if (check_ot_time_rollover(current_window)) // Manual mode
				{
					clear_window_fsm_input_flags(&(current_window -> flags));
					turn_off_window_OT_timer(&(current_window -> flags));
					set_output(current_window, OUTPUT_REQUEST.UP_RELEASED_STOP);
					current_window -> next_state = WINDOW_FSM_STATE.IDLE;
					break;
				}
				else // Auto mode
				{
					clear_window_fsm_input_flags(&(current_window -> flags));
					turn_off_window_OT_timer(&(current_window -> flags));
					set_output(current_window, OUTPUT_REQUEST.UP_RELEASED_AUTO);
					current_window -> next_state = WINDOW_FSM_STATE.IDLE;
					break;
				}
            }
            if (check_up_and_down(current_window)) //Auto mode double-click button
			{
                clear_window_fsm_input_flags(&(current_window -> flags));
                turn_off_window_OT_timer(&(current_window -> flags));
                current_window->next_state = WINDOW_FSM_STATE.AUTO_UP;
                break;
            }
			clear_window_fsm_input_flags(&(current_window -> flags));
			turn_off_window_OT_timer(&(current_window -> flags));
			set_output(current_window, OUTPUT_REQUEST.UP_RELEASED_STOP);
			current_window -> next_state = WINDOW_FSM_STATE.BLOCKED;
			break;
        }
        case WINDOW_FSM_STATE.AUTO_DOWN:
        {			
            if (check_down(current_window)) // Keep polling
			{
                clear_window_fsm_input_flags(&(current_window->flags));
                current_window -> next_state = WINDOW_FSM_STATE.AUTO_DOWN;
                break;
            }

            if (check_no_up_and_no_down(current_window)) // Button fully released
			{
                clear_window_fsm_input_flags(&(current_window->flags));
                set_output(current_window, OUTPUT_REQUEST.DOWN_RELEASED_AUTO);
                current_window -> next_state = WINDOW_FSM_STATE.IDLE;
                break;
            }
			clear_window_fsm_input_flags(&(current_window -> flags));
			turn_off_window_OT_timer(&(current_window -> flags));
			set_output(current_window, OUTPUT_REQUEST.DOWN_RELEASED_STOP);
			current_window -> next_state = WINDOW_FSM_STATE.BLOCKED;
			break;
        }
        case WINDOW_FSM_STATE.AUTO_UP:
        {
	        if (check_up(current_window)) // Keep polling
	        {
		        clear_window_fsm_input_flags(&(current_window->flags));
		        current_window -> next_state = WINDOW_FSM_STATE.AUTO_UP;
		        break;
	        }

	        if (check_no_up_and_no_down(current_window)) // Button fully released
	        {
		        clear_window_fsm_input_flags(&(current_window->flags));
		        set_output(current_window, OUTPUT_REQUEST.UP_RELEASED_AUTO);
		        current_window -> next_state = WINDOW_FSM_STATE.IDLE;
		        break;
	        }
	        clear_window_fsm_input_flags(&(current_window -> flags));
	        turn_off_window_OT_timer(&(current_window -> flags));
	        set_output(current_window, OUTPUT_REQUEST.UP_RELEASED_STOP);
	        current_window -> next_state = WINDOW_FSM_STATE.BLOCKED;
	        break;
        } 
        case WINDOW_FSM_STATE.BLOCKED: // Wrong button sequence stops controller untill all buttons are released
		{            
            turn_off_window_OT_timer(&(current_window -> flags));
            if (check_any_pressed(current_window))
			{
				clear_window_fsm_input_flags(&(current_window -> flags));
				current_window -> next_state = WINDOW_FSM_STATE.BLOCKED;
			}
			else
			{
				clear_window_fsm_input_flags(&(current_window -> flags));
				current_window -> next_state = WINDOW_FSM_STATE.IDLE;
			} 
            break;
        }
        case WINDOW_FSM_STATE.CENTRAL_CLOSE: //Handles second part of button sequence for commanding auto-lift mode
		{
			clear_window_fsm_input_flags(&(current_window -> flags));
			set_output(current_window, OUTPUT_REQUEST.UP_RELEASED_AUTO);
			current_window -> next_state = WINDOW_FSM_STATE.AUTHORIZATION_OFF;
			break;
        }
		case WINDOW_FSM_STATE.AUTHORIZATION_OFF: // Disables authorization override signal once auto-lift sequence is over (and thus valid)
		{
			if (check_central_close_over(current_window)) // Required as up and down lines and authorization line are managed independently
			{
				clear_window_fsm_input_flags(&(current_window -> flags));
				set_authorization_request_flags(&(current_window->flags), OFF);
				current_window -> next_state = WINDOW_FSM_STATE.IDLE;
			}
			else
			{		
				clear_window_fsm_input_flags(&(current_window -> flags));		
				current_window -> next_state = WINDOW_FSM_STATE.AUTHORIZATION_OFF;	
			}			
			break;
		}
    }
}

inline void windows_fsm_fire_all()
{
    unsigned char i;
    for(i = 0; i < NUM_WINDOWS; ++i)
	{
        window_fsm_fire(&windows[i]);
    }
}

void authorization_fsm_fire(window_t* current_window)
{
	current_window -> authorizationState = current_window -> authorizationNextState; // Updates state	
	switch (current_window -> authorizationState)
	{
		default:
		case AUTHORIZATION_FSM_STATE.OFF:
		{
			if (!check_authorization_input(current_window) && //OFF
				!check_authorization_override_request(current_window))
			{
				clear_authorization_fsm_input_flags(&(current_window -> flags));
				//turn auth override off
				write_authorization(current_window, OFF);
				current_window -> authorizationNextState = AUTHORIZATION_FSM_STATE.OFF;
				break;
			}
			if (check_authorization_input(current_window)) // Auth on
			{
				clear_authorization_fsm_input_flags(&(current_window -> flags));
				//turn auth override on
				write_authorization(current_window, ON);
				current_window -> authorizationNextState = AUTHORIZATION_FSM_STATE.ARMED;
				break;
			}
			if (!check_authorization_input(current_window) && 
				check_authorization_override_request(current_window))
			{
				clear_authorization_fsm_input_flags(&(current_window -> flags));
				//turn auth override on
				write_authorization(current_window, ON);
				set_authorization_timer(current_window, AUTHORIZATION_EXTENSION_TIME_MS);
				current_window -> authorizationNextState = AUTHORIZATION_FSM_STATE.OVERRIDE;
				break;
			}			
		}
		case AUTHORIZATION_FSM_STATE.ARMED:
		{
			if (check_authorization_input(current_window)) // Auth on
			{
				clear_authorization_fsm_input_flags(&(current_window -> flags));
				//turn auth override on
				write_authorization(current_window, ON);
				current_window -> authorizationNextState = AUTHORIZATION_FSM_STATE.ARMED;
				break;
			}
			else
			{
				clear_authorization_fsm_input_flags(&(current_window -> flags));
				//turn auth override on
				write_authorization(current_window, ON);
				set_authorization_timer(current_window, AUTHORIZATION_EXTENSION_TIME_MS);
				current_window -> authorizationNextState = AUTHORIZATION_FSM_STATE.OVERRIDE;
				break;
			}			
		}
		case AUTHORIZATION_FSM_STATE.OVERRIDE:
		{
			if (check_authorization_input(current_window)) //BCM auth returns
			{
				clear_authorization_fsm_input_flags(&(current_window -> flags));
				//turn auth override on
				write_authorization(current_window, ON);
				turn_off_authorization_timer(&(current_window -> flags));
				current_window -> authorizationNextState = AUTHORIZATION_FSM_STATE.ARMED;
				break;
			}
			if (check_authorization_override_request(current_window)) // Central close request, timeout restarts
			{
				clear_authorization_fsm_input_flags(&(current_window -> flags));
				//turn auth override on
				write_authorization(current_window, ON);
				set_authorization_timer(current_window, AUTHORIZATION_EXTENSION_TIME_MS);
				current_window -> authorizationNextState = AUTHORIZATION_FSM_STATE.OVERRIDE;
				break;
			}
			if (!check_authorization_input(current_window) && // Signal can return to offf
				(check_authorization_time_rollover(current_window) || check_authorization_release_request(current_window)))
			{
				clear_authorization_fsm_input_flags(&(current_window -> flags));
				//turn auth override off
				write_authorization(current_window, OFF);
				turn_off_authorization_timer(&(current_window -> flags));
				current_window -> authorizationNextState = AUTHORIZATION_FSM_STATE.OFF;
				break;
			}
			if (!check_authorization_time_rollover(current_window)) && // Timeout running
				!check_authorization_input(current_window) &&
				!check_authorization_release_request(current_window))
			{
				clear_authorization_fsm_input_flags(&(current_window -> flags));
				//turn auth override on
				write_authorization(current_window, ON);
				current_window -> authorizationNextState = AUTHORIZATION_FSM_STATE.OVERRIDE;
				break;
			}
		}
	}
}


void windows_init(void)
{
    unsigned char i;
    for (i = 0; i < NUM_WINDOWS; i++) 
	{
		// Window FSM
        windows[i].current_state = WINDOW_FSM_STATE.IDLE;
        windows[i].next_state = WINDOW_FSM_STATE.IDLE;
        windows[i].ot_timer_counter = 0;
        windows[i].flags.up_sw = OFF;
        windows[i].flags.down_sw = OFF;
		windows[i].flags.up_rem_sw = OFF;
		windows[i].flags.down_rem_sw = OFF;
		windows[i].flags.cclose_in = OFF;
		windows[i].flags.cclose_over = OFF;
        windows[i].flags.ot_timer_rollover = OFF;
        windows[i].flags.ot_timer_enable = OFF;
		// Output FSM	
		windows[i].outputState = OUTPUT_FSM_STATE.IDLE;
		windows[i].outputNextState = OUTPUT_FSM_STATE.IDLE;
		windows[i].output_timer_counter = 0;
		windows[i].output = OUTPUT_REQUEST.NONE;
		windows[i].flags.output_timer_rollover = OFF;
		windows[i].flags.output_timer_enable = OFF;
		// Authorization FSM
		windows[i].authorizationState = AUTHORIZATION_FSM_STATE.OFF;
		windows[i].authorizationNextState = AUTHORIZATION_FSM_STATE.OFF;
		windows[i].authorization_timer_counter = 0;
		windows[i].flags.authorization_in = OFF;
		windows[i].flags.authorization_on = OFF;
		windows[i].flags.authorization_off = OFF;
		windows[i].flags.authorization_timer_rollover = OFF;
		windows[i].flags.authorization_timer_enable = OFF;
    }
}



window_t get_window(unsigned char window)
{    
        return windows[window];
}

window_t* get_window_pointer (unsigned char window)
{
        return (windows+window);
}

unsigned char set_window_id(unsigned char window_index, unsigned char id)
{
    if (window_index < NUM_WINDOWS)
	{
        windows[window_index].id = id;
        return 1;
    }
    return 0;
}

// Window FSM output functions
inline void set_output(window_t* current_window, OUTPUT_REQUEST o)
{
	(current_window -> output) = o;
}

inline void set_window_OT_timer(window_t* current_window, int time)
{
	current_window -> ot_timer_max_count = time;
	current_window -> ot_timer_counter = 0;
	current_window -> flags.ot_timer_rollover = 0;
	current_window -> flags.ot_timer_enable = ON;
	return;
}

inline void turn_off_window_OT_timer(volatile FLAGS* flags)
{
	flags -> ot_timer_enable = OFF;
	return;
}

inline void set_authorization_request_flags(window_t* current_window, unsigned char auth_state)
{
	if (auth_state)
	{
		(current_window -> flags).authorization_on = 1;
		(current_window -> flags).authorization_off = 0;
	}
	else
	{
		(current_window -> flags).authorization_on = 0;
		(current_window -> flags).authorization_off = 1;
	}
}

// Window FSM input functions
inline void clear_window_fsm_input_flags(volatile FLAGS* flags)
{
	flags -> down_sw = 0;
	flags -> up_sw = 0;
	flags -> up_rem_sw = 0;
	flags -> down_rem_sw = 0;
	flags -> cclose_in = 0;
	flags -> cclose_over = 0;
	flags -> ot_timer_rollover = 0;
	return;
}

unsigned char check_up_and_no_down(window_t* current_window) //yellow
{
	return (((current_window->flags).up_sw && !(current_window->flags).down_sw) || 
			((current_window->flags).up_rem_sw && !(current_window->flags).down_rem_sw));
}

unsigned char check_down_and_no_up(window_t* current_window) //green
{
	return ((!(current_window->flags).up_sw && (current_window->flags).down_sw) ||
			(!(current_window->flags).up_rem_sw && (current_window->flags).down_rem_sw));
}

unsigned char check_up_and_down(window_t* current_window) //orange
{
	return (((current_window->flags).up_sw && (current_window->flags).down_sw) ||
			((current_window->flags).up_rem_sw && (current_window->flags).down_rem_sw));
}

unsigned char check_all_released(window_t* current_window) // blue
{
	return (!(current_window->flags).up_sw &&
			!(current_window->flags).down_sw &&
			!(current_window->flags).up_rem_sw && 
			!(current_window->flags).down_rem_sw);
}

unsigned char check_any_pressed(window_t* current_window) //red
{
	return ((current_window->flags).up_sw || 
			(current_window->flags).down_sw ||
			(current_window->flags).up_rem_sw ||
			(current_window->flags).down_rem_sw);
}

unsigned char check_no_up_and_no_down(window_t* current_window) // gray
{
	return ((!(current_window->flags).up_sw && !(current_window->flags).down_sw) ||
			(!(current_window->flags).up_rem_sw && !(current_window->flags).down_rem_sw));
}

unsigned char check_up(window_t* current_window) //pink
{
	return ((current_window->flags).up_sw || 
			(current_window->flags).up_rem_sw);
}

unsigned char check_down(window_t* current_window) // same as pink for down
{
	return ((current_window->flags).down_sw ||
			(current_window->flags).down_rem_sw);
}

unsigned char check_ot_time_rollover(window_t* current_window)
{
	return (current_window -> flags).ot_timer_rollover;
}

unsigned char check_central_close(window_t* current_window)
{
	return (current_window -> flags).cclose_in;
}

unsigned char check_central_close_over(window_t* current_window)
{
	return (current_window -> flags).cclose_over;
}

//AUTH FSM
inline void clear_authorization_fsm_input_flags(volatile FLAGS* flags)
{
	flags -> authorization_in = 0;
	flags -> authorization_on = 0;
	flags -> authorization_off = 0;
	flags -> authorization_timer_rollover = 0;
}

unsigned char check_authorization_input(window_t* current_window)
{
	return (current_window -> flags).authorization_in;
}

unsigned char check_authorization_override_request(window_t* current_window)
{
	return (current_window -> flags).authorization_on;
}

unsigned char check_authorization_release_request(window_t* current_window)
{
	return (current_window -> flags).authorization_off;
}

unsigned char check_authorization_time_rollover(window_t* current_window)
{
	return (current_window -> flags).authorization_timer_rollover;
}

inline void write_authorization(window_t* current_window, unsigned char level)
{
	// TODO
	// Write GPIO
}

inline void set_authorization_timer(window_t* current_window, int time)
{
	current_window -> authorization_timer_max_count = time;
	current_window -> authorization_timer_counter = 0;
	current_window -> flags.authorization_timer_rollover = 0;
	current_window -> flags.authorization_timer_enable = ON;
	return;
}

inline void turn_off_authorization_timer(volatile FLAGS* flags)
{
	flags -> authorization_timer_enable = OFF;
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

void read_port(void)
{
	// TODO
	// Update and add mismatch
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
    for (i = 0; i < NUM_WINDOWS; i++) 
	{
        if (windows[i].ot_timer_counter >= windows[i].ot_timer_max_count)
		{
            windows[i].flags.ot_timer_rollover = ON;
        }
    }
}

volatile void timer_interrupt(void)
{
    if (TIMER_IF && TIMER_IE) //remove PIC crap
	{
        TIMER_IF = 0;
        unsigned char i;
        for (i = 0; i < NUM_WINDOWS; ++i)
		{
            if(windows[i].flags.authorization_timer_enable)
			{
                windows[i].authorization_timer_counter++;
            }
            if(windows[i].flags.ot_timer_enable )
			{
                windows[i].ot_timer_counter++;
            }
            if (windows[i].flags.output_timer_enable)
			{
                windows[i].output_timer_counter++;
            }
        }
    }
}











