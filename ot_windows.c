
#include "ot_windows.h"
#include "ports.h"
#include "timer.h"

#define DEBUG_MODE

volatile unsigned char timer_flag;

void ot_window_init(window_t* current_window)
{
	initDebugLine();
	// GPIO init
	port_init(current_window);	
	// Timer init
	timer_init(TIMER0_PRESCALER_FOR_1MS_TICK_WITH_16MHZ, TIMER0_COUNT_FOR_1MS_TICK_WITH_16MHZ);
	// Input init
	current_window -> input_timer_counter = 0;	
	current_window -> flags.up_sw_last = OFF;
	current_window -> flags.down_sw_last = OFF;
	current_window -> flags.up_rem_sw_last = OFF;
	current_window -> flags.down_rem_sw_last = OFF;
	current_window -> flags.cclose_in_last = OFF;
	current_window -> flags.authorization_in_last = OFF;	
	current_window -> input_timer_max_count = INPUT_TIME_FILTER_MS;	
	current_window -> flags.input_timer_rollover = OFF;
	current_window -> flags.input_timer_enable = ON; // Start timer now
	// Window FSM
	current_window -> current_state = IDLE;
	current_window -> next_state = IDLE;
	current_window -> ot_timer_counter = 0;
	current_window -> flags.up_sw = OFF;
	current_window -> flags.down_sw = OFF;
	current_window -> flags.up_rem_sw = OFF;
	current_window -> flags.down_rem_sw = OFF;
	current_window -> flags.cclose_in = OFF;
	current_window -> flags.cclose_over = OFF;
	current_window -> flags.ot_timer_rollover = OFF;
	current_window -> flags.ot_timer_enable = OFF;
	// Output FSM
	current_window -> outputState = ZERO;
	current_window -> outputNextState = ZERO;
	current_window -> output_timer_counter = 0;
	current_window -> output = NONE;
	current_window -> flags.output_timer_rollover = OFF;
	current_window -> flags.output_timer_enable = OFF;
	// Authorization FSM
	current_window -> authorizationState = RELEASED;
	current_window -> authorizationNextState = RELEASED;
	current_window -> authorization_timer_counter = 0;
	current_window -> flags.authorization_in = OFF;
	current_window -> flags.authorization_on = OFF;
	current_window -> flags.authorization_off = OFF;
	current_window -> flags.authorization_timer_rollover = OFF;
	current_window -> flags.authorization_timer_enable = OFF;	
}

void ot_window_run(window_t* current_window)
{		
	set_timer_flags(current_window);
	read_port(current_window);	
	window_fsm_fire(current_window);
	authorization_fsm_fire(current_window);
	output_fsm_fire(current_window);		
}

void port_init(window_t* current_window)
{
	// Output pins: off
	UP_OUT_PORT &= ~(1 << UP_OUT_BIT_POS);
	UP_OUT_DDR |= (1 << UP_OUT_BIT_POS);
	DOWN_OUT_PORT &= ~(1 << DOWN_OUT_BIT_POS);
	DOWN_OUT_DDR |= (1 << DOWN_OUT_BIT_POS);
	AUTH_OUT_PORT &= ~(1 << AUTH_OUT_BIT_POS);
	AUTH_OUT_DDR |= (1 << AUTH_OUT_BIT_POS);
	// Input pins: no pull up/down
	UP_IN_PORT &= ~(1 << UP_IN_BIT_POS);
	UP_IN_DDR &= ~(1 << UP_IN_BIT_POS);
	DOWN_IN_PORT &= ~(1 << DOWN_IN_BIT_POS);
	DOWN_IN_DDR &= ~(1 << DOWN_IN_BIT_POS);
	UP_REM_PORT &= ~(1 << UP_REM_BIT_POS);
	UP_REM_DDR &= ~(1 << UP_REM_BIT_POS);
	DOWN_REM_PORT &= ~(1 << DOWN_REM_BIT_POS);
	DOWN_REM_DDR &= ~(1 << DOWN_REM_BIT_POS);
	AUTH_IN_PORT &= ~(1 << AUTH_IN_BIT_POS);
	AUTH_IN_DDR &= ~(1 << AUTH_IN_BIT_POS);
	CCLOSE_IN_PORT &= ~(1 << CCLOSE_IN_BIT_POS);
	CCLOSE_IN_DDR &= ~(1 << CCLOSE_IN_BIT_POS);	
}

void timer_init(unsigned char prescaler, unsigned char count)
{
	timer_flag = 0;
	TCCR0A=0b00000010;							//CTC with compare match threshold on OCRA. No HW pin toggling.
	OCR0A=count;								//Set threshold
	TCNT0=0;									//Clear count
	TIFR0|=	0b00000010;							//Clear flag
	TIMSK0|= 0b00000010;						//Interrupt enable
	TCCR0B=prescaler & 0b00000111;				//Timer on
}

void set_timer_flags(window_t* current_window)
{
	if (timer_flag)
	{
		if(current_window -> flags.authorization_timer_enable)
		{
			current_window -> authorization_timer_counter++;
			if (current_window -> authorization_timer_counter >= current_window -> authorization_timer_max_count)
			{
				current_window -> flags.authorization_timer_rollover = 1;
			}
		}
		if(current_window -> flags.ot_timer_enable )
		{
			current_window -> ot_timer_counter++;
			if (current_window -> ot_timer_counter >= current_window -> ot_timer_max_count)
			{
				current_window -> flags.ot_timer_rollover = 1;
			}
		}
		if (current_window -> flags.output_timer_enable)
		{
			current_window -> output_timer_counter++;
			if (current_window -> output_timer_counter >= current_window -> output_timer_max_count)
			{
				current_window -> flags.output_timer_rollover = 1;
			}
		}
		if (current_window -> flags.input_timer_enable)
		{
			current_window -> input_timer_counter++;
			if (current_window -> input_timer_counter >= current_window -> input_timer_max_count)
			{
				current_window -> flags.input_timer_rollover = 1;
			}
		}
		timer_flag = 0;		
	}
}

void timer_interrupt(void)
{
	timer_flag = 1;
}

void read_port(window_t* current_window)
{
	// TODO
	// Update and add mismatch??
	
	// Automotive line is active low
	// Input stage pushes GPIO high when automotive line is low
	// Flag variable is set ON when automotive line is asserted and low
	// Therefore, if GPIO is high flag shall be set ON
	if (current_window -> flags.input_timer_rollover)
	{
		// If last value matches current
		if (((AUTH_IN_PIN & (1 << AUTH_IN_BIT_POS)) && (current_window -> flags.authorization_in_last)) ||
		(!(AUTH_IN_PIN & (1 << AUTH_IN_BIT_POS)) && !(current_window -> flags.authorization_in_last)))
		{
			// output is updated to current/last
			if (AUTH_IN_PIN & (1 << AUTH_IN_BIT_POS))
			{
				current_window -> flags.authorization_in = ON;
			}
			else
			{
				current_window -> flags.authorization_in = OFF;
			}
		}
		// Update last value
		if (AUTH_IN_PIN & (1 << AUTH_IN_BIT_POS))
		{
			current_window -> flags.authorization_in_last = ON;
		}
		else
		{
			current_window -> flags.authorization_in_last = OFF;
		}
		// If last value matches current
		if (((CCLOSE_IN_PIN & (1 << CCLOSE_IN_BIT_POS)) && (current_window -> flags.cclose_in_last)) ||
		(!(CCLOSE_IN_PIN & (1 << CCLOSE_IN_BIT_POS)) && !(current_window -> flags.cclose_in_last)))
		{
			// output is updated to current/last
			if (CCLOSE_IN_PIN & (1 << CCLOSE_IN_BIT_POS))
			{
				current_window -> flags.cclose_in = ON;
			}
			else
			{
				current_window -> flags.cclose_in = OFF;
			}
		}
		// Update last value
		if (CCLOSE_IN_PIN & (1 << CCLOSE_IN_BIT_POS))
		{
			current_window -> flags.cclose_in_last = ON;
		}
		else
		{
			current_window -> flags.cclose_in_last = OFF;
		}
		// If last value matches current
		if (((UP_IN_PIN & (1 << UP_IN_BIT_POS)) && (current_window -> flags.up_sw_last)) ||
		(!(UP_IN_PIN & (1 << UP_IN_BIT_POS)) && !(current_window -> flags.up_sw_last)))
		{
			// output is updated to current/last
			if (UP_IN_PIN & (1 << UP_IN_BIT_POS))
			{
				current_window -> flags.up_sw = ON;
			}
			else
			{
				current_window -> flags.up_sw = OFF;
			}
		}
		// Update last value
		if (UP_IN_PIN & (1 << UP_IN_BIT_POS))
		{
			current_window -> flags.up_sw_last = ON;
		}
		else
		{
			current_window -> flags.up_sw_last = OFF;
		}
		// If last value matches current
		if (((DOWN_IN_PIN & (1 << DOWN_IN_BIT_POS)) && (current_window -> flags.down_sw_last)) ||
		(!(DOWN_IN_PIN & (1 << DOWN_IN_BIT_POS)) && !(current_window -> flags.down_sw_last)))
		{
			// output is updated to current/last
			if (DOWN_IN_PIN & (1 << DOWN_IN_BIT_POS))
			{
				current_window -> flags.down_sw = ON;
			}
			else
			{
				current_window -> flags.down_sw = OFF;
			}
		}
		// Update last value
		if (DOWN_IN_PIN & (1 << DOWN_IN_BIT_POS))
		{
			current_window -> flags.down_sw_last = ON;
		}
		else
		{
			current_window -> flags.down_sw_last = OFF;
		}
		// If last value matches current
		if (((UP_IN_PIN & (1 << UP_IN_BIT_POS)) && (current_window -> flags.up_sw_last)) ||
		(!(UP_IN_PIN & (1 << UP_IN_BIT_POS)) && !(current_window -> flags.up_sw_last)))
		{
			// output is updated to current/last
			if (UP_IN_PIN & (1 << UP_IN_BIT_POS))
			{
				current_window -> flags.up_sw = ON;
			}
			else
			{
				current_window -> flags.up_sw = OFF;
			}
		}
		// Update last value
		if (UP_IN_PIN & (1 << UP_IN_BIT_POS))
		{
			current_window -> flags.up_sw_last = ON;
		}
		else
		{
			current_window -> flags.up_sw_last = OFF;
		}

		// If last value matches current
		if (((DOWN_IN_PIN & (1 << DOWN_IN_BIT_POS)) && (current_window -> flags.down_sw_last)) ||
		(!(DOWN_IN_PIN & (1 << DOWN_IN_BIT_POS)) && !(current_window -> flags.down_sw_last)))
		{
			// output is updated to current/last
			if (DOWN_IN_PIN & (1 << DOWN_IN_BIT_POS))
			{
				current_window -> flags.down_sw = ON;
			}
			else
			{
				current_window -> flags.down_sw = OFF;
			}
		}
		// Update last value
		if (DOWN_IN_PIN & (1 << DOWN_IN_BIT_POS))
		{
			current_window -> flags.down_sw_last = ON;
		}
		else
		{
			current_window -> flags.down_sw_last = OFF;
		}
		// If last value matches current
		if (((UP_REM_PIN & (1 << UP_REM_BIT_POS)) && (current_window -> flags.up_rem_sw_last)) ||
		(!(UP_REM_PIN & (1 << UP_REM_BIT_POS)) && !(current_window -> flags.up_rem_sw_last)))
		{
			// output is updated to current/last
			if (UP_REM_PIN & (1 << UP_REM_BIT_POS))
			{
				current_window -> flags.up_rem_sw = ON;
			}
			else
			{
				current_window -> flags.up_rem_sw = OFF;
			}
		}
		// Update last value
		if (UP_REM_PIN & (1 << UP_REM_BIT_POS))
		{
			current_window -> flags.up_rem_sw_last = ON;
		}
		else
		{
			current_window -> flags.up_rem_sw_last = OFF;
		}

		// If last value matches current
		if (((DOWN_REM_PIN & (1 << DOWN_REM_BIT_POS)) && (current_window -> flags.down_rem_sw_last)) ||
		(!(DOWN_REM_PIN & (1 << DOWN_REM_BIT_POS)) && !(current_window -> flags.down_rem_sw_last)))
		{
			// output is updated to current/last
			if (DOWN_REM_PIN & (1 << DOWN_REM_BIT_POS))
			{
				current_window -> flags.down_rem_sw = ON;
			}
			else
			{
				current_window -> flags.down_rem_sw = OFF;
			}
		}
		// Update last value
		if (DOWN_REM_PIN & (1 << DOWN_REM_BIT_POS))
		{
			current_window -> flags.down_rem_sw_last = ON;
		}
		else
		{
			current_window -> flags.down_rem_sw_last = OFF;
		}
		current_window -> input_timer_counter = 0;
		current_window -> input_timer_max_count = INPUT_TIME_FILTER_MS;
		current_window -> flags.input_timer_rollover = OFF;
		current_window -> flags.input_timer_enable = ON;
	}
}

void window_fsm_fire(window_t* current_window)
{
    current_window -> current_state = current_window -> next_state; // Updates state
	
	switch (current_window -> current_state)
	{		
		default:
		case IDLE:
        {	
			if (check_all_released(current_window) && !check_central_close(current_window)) // Idle
			{                
                clear_window_fsm_input_flags(&(current_window -> flags));
				turn_off_window_OT_timer(&(current_window -> flags));             
                current_window -> next_state = IDLE;
				break;
            }
			if (check_all_released(current_window) && check_central_close(current_window)) // Central close
			{
				clear_window_fsm_input_flags(&(current_window->flags));
				set_authorization_request_flags(current_window, ON);
				set_output(current_window, UP_PRESSED);
				current_window -> next_state = CENTRAL_CLOSE;
				break;				
			}
            if (check_up_and_no_down(current_window)) //Up botton pressed
			{
                clear_window_fsm_input_flags(&(current_window->flags));
				set_window_OT_timer(current_window, OT_TIMER_COUNT_MS);
                set_output(current_window, UP_PRESSED);
                current_window -> next_state = MAN_UP;
				break;
            }
            if (check_down_and_no_up(current_window)) //Down botton pressed
			{
                clear_window_fsm_input_flags(&(current_window->flags));
				set_window_OT_timer(current_window, OT_TIMER_COUNT_MS);
                set_output(current_window, DOWN_PRESSED);           
                current_window -> next_state = MAN_DOWN;
				break;
            }
			clear_window_fsm_input_flags(&(current_window->flags));
			turn_off_window_OT_timer(&(current_window -> flags));			
			set_output(current_window, NONE);
            current_window -> next_state = BLOCKED;
			break;			
        }		
		case MAN_DOWN:
		{
			if (check_mismatch(current_window))
			{
				clear_window_fsm_input_flags(&(current_window -> flags));
				turn_off_window_OT_timer(&(current_window -> flags));
				set_output(current_window, DOWN_RELEASED_STOP);
				current_window -> next_state = BLOCKED;
				break;
			}						
			if (check_down_and_no_up(current_window))
			{
				clear_window_fsm_input_flags(&(current_window->flags));
				current_window->next_state = MAN_DOWN;
				break;
			}
			if (check_all_released(current_window))
			{
				if (check_ot_time_rollover(current_window)) // Manual mode
				{
					clear_window_fsm_input_flags(&(current_window -> flags));
					turn_off_window_OT_timer(&(current_window -> flags));
					set_output(current_window, DOWN_RELEASED_STOP);
					current_window -> next_state = IDLE;
					break;
				}
				else // Auto mode
				{
					clear_window_fsm_input_flags(&(current_window -> flags));
					turn_off_window_OT_timer(&(current_window -> flags));
					set_output(current_window, DOWN_RELEASED_AUTO);
					current_window -> next_state = IDLE;
					break;
				}
			}
			if (check_up_and_down(current_window)) //Auto mode double-click button
			{
				clear_window_fsm_input_flags(&(current_window -> flags));
				turn_off_window_OT_timer(&(current_window -> flags));
				current_window->next_state = AUTO_DOWN;
				break;
			}
			clear_window_fsm_input_flags(&(current_window -> flags));
			turn_off_window_OT_timer(&(current_window -> flags));
			set_output(current_window, DOWN_RELEASED_STOP);
			current_window -> next_state = BLOCKED;
			break;
		}		
        case MAN_UP:
        {	
			if (check_mismatch(current_window))
			{
				clear_window_fsm_input_flags(&(current_window -> flags));
				turn_off_window_OT_timer(&(current_window -> flags));
				set_output(current_window, UP_RELEASED_STOP);
				current_window -> next_state = BLOCKED;
				break;
			}						
            if (check_up_and_no_down(current_window))
			{
                clear_window_fsm_input_flags(&(current_window->flags));
                current_window->next_state = MAN_UP;
                break;
            }
            if (check_all_released(current_window))
			{
				if (check_ot_time_rollover(current_window)) // Manual mode
				{
					clear_window_fsm_input_flags(&(current_window -> flags));
					turn_off_window_OT_timer(&(current_window -> flags));
					set_output(current_window, UP_RELEASED_STOP);
					current_window -> next_state = IDLE;
					break;
				}
				else // Auto mode
				{
					clear_window_fsm_input_flags(&(current_window -> flags));
					turn_off_window_OT_timer(&(current_window -> flags));
					set_output(current_window, UP_RELEASED_AUTO);
					current_window -> next_state = IDLE;
					break;
				}
            }
            if (check_up_and_down(current_window)) //Auto mode double-click button
			{
                clear_window_fsm_input_flags(&(current_window -> flags));
                turn_off_window_OT_timer(&(current_window -> flags));
                current_window->next_state = AUTO_UP;
                break;
            }
			clear_window_fsm_input_flags(&(current_window -> flags));
			turn_off_window_OT_timer(&(current_window -> flags));
			set_output(current_window, UP_RELEASED_STOP);
			current_window -> next_state = BLOCKED;
			break;
        }
        case AUTO_DOWN:
        {	
			if (check_mismatch_auto_down(current_window))								
			{
				clear_window_fsm_input_flags(&(current_window -> flags));
				turn_off_window_OT_timer(&(current_window -> flags));
				set_output(current_window, DOWN_RELEASED_STOP);
				current_window -> next_state = BLOCKED;
				break;
			}
            if (check_down(current_window)) // Keep polling
			{
                clear_window_fsm_input_flags(&(current_window->flags));
                current_window -> next_state = AUTO_DOWN;
                break;
            }

            if (check_no_up_and_no_down(current_window)) // Button fully released
			{
                clear_window_fsm_input_flags(&(current_window->flags));
                set_output(current_window, DOWN_RELEASED_AUTO);
                current_window -> next_state = IDLE;
                break;
            }
			clear_window_fsm_input_flags(&(current_window -> flags));
			turn_off_window_OT_timer(&(current_window -> flags));
			set_output(current_window, DOWN_RELEASED_STOP);
			current_window -> next_state = BLOCKED;
			break;
        }
        case AUTO_UP:
        {		
			if (check_mismatch_auto_up(current_window))
			{
				clear_window_fsm_input_flags(&(current_window -> flags));
				turn_off_window_OT_timer(&(current_window -> flags));
				set_output(current_window, UP_RELEASED_STOP);
				current_window -> next_state = BLOCKED;
				break;
			}			
	        if (check_up(current_window)) // Keep polling
	        {
		        clear_window_fsm_input_flags(&(current_window->flags));
		        current_window -> next_state = AUTO_UP;
		        break;
	        }

	        if (check_no_up_and_no_down(current_window)) // Button fully released
	        {
		        clear_window_fsm_input_flags(&(current_window->flags));
		        set_output(current_window, UP_RELEASED_AUTO);
		        current_window -> next_state = IDLE;
		        break;
	        }
	        clear_window_fsm_input_flags(&(current_window -> flags));
	        turn_off_window_OT_timer(&(current_window -> flags));
	        set_output(current_window, UP_RELEASED_STOP);
	        current_window -> next_state = BLOCKED;
	        break;
        } 
        case BLOCKED: // Wrong button sequence stops controller untill all buttons are released
		{  					     
			turn_off_window_OT_timer(&(current_window -> flags));
            if (check_any_pressed(current_window))
			{
				clear_window_fsm_input_flags(&(current_window -> flags));
				current_window -> next_state = BLOCKED;
			}
			else
			{
				clear_window_fsm_input_flags(&(current_window -> flags));
				current_window -> next_state = IDLE;
			} 
            break;
        }
        case CENTRAL_CLOSE: //Handles second part of button sequence for commanding auto-lift mode
		{					
			clear_window_fsm_input_flags(&(current_window -> flags));
			set_output(current_window, UP_RELEASED_AUTO);
			current_window -> next_state = AUTHORIZATION_OFF;
			break;
        }
		case AUTHORIZATION_OFF: // Disables authorization override signal once auto-lift sequence is over (and thus valid)
		{
			if (check_central_close_over(current_window) && // Required as up and down lines and authorization line are managed independently
				check_no_central_close(current_window))  // To avoid retriggers while cclose line is low
			{
				clear_window_fsm_input_flags(&(current_window -> flags));
				set_authorization_request_flags(current_window, OFF);
				current_window -> next_state = IDLE;
			}
			else
			{		
				clear_window_fsm_input_flags(&(current_window -> flags));		
				current_window -> next_state = AUTHORIZATION_OFF;	
			}			
			break;
		}
    }
}

void authorization_fsm_fire(window_t* current_window)
{
	current_window -> authorizationState = current_window -> authorizationNextState; // Updates state	
	switch (current_window -> authorizationState)
	{
		default:
		case RELEASED:
		{
			if (!check_authorization_input(current_window) && //OFF
				!check_authorization_override_request(current_window))
			{
				clear_authorization_fsm_input_flags(&(current_window -> flags));
				//turn auth override off
				write_authorization(current_window, OFF);
				current_window -> authorizationNextState = RELEASED;
				break;
			}
			if (check_authorization_input(current_window)) // Auth on
			{
				clear_authorization_fsm_input_flags(&(current_window -> flags));
				//turn auth override on
				write_authorization(current_window, ON);
				current_window -> authorizationNextState = ARMED;
				break;
			}
			if (!check_authorization_input(current_window) && 
				check_authorization_override_request(current_window))
			{
				clear_authorization_fsm_input_flags(&(current_window -> flags));
				//turn auth override on
				write_authorization(current_window, ON);
				set_authorization_timer(current_window, AUTHORIZATION_EXTENSION_TIME_MS);
				current_window -> authorizationNextState = OVERRIDE;
				break;
			}			
		}
		case ARMED:
		{
			if (check_authorization_input(current_window)) // Auth on
			{
				clear_authorization_fsm_input_flags(&(current_window -> flags));
				//turn auth override on
				write_authorization(current_window, ON);
				current_window -> authorizationNextState = ARMED;
				break;
			}
			else
			{
				clear_authorization_fsm_input_flags(&(current_window -> flags));
				//turn auth override on
				write_authorization(current_window, ON);
				set_authorization_timer(current_window, AUTHORIZATION_EXTENSION_TIME_MS);
				current_window -> authorizationNextState = OVERRIDE;
				break;
			}			
		}
		case OVERRIDE:
		{
			if (check_authorization_input(current_window)) //BCM auth returns
			{
				clear_authorization_fsm_input_flags(&(current_window -> flags));
				//turn auth override on
				write_authorization(current_window, ON);
				turn_off_authorization_timer(&(current_window -> flags));
				current_window -> authorizationNextState = ARMED;
				break;
			}
			if (check_authorization_override_request(current_window)) // Central close request, timeout restarts
			{
				clear_authorization_fsm_input_flags(&(current_window -> flags));
				//turn auth override on
				write_authorization(current_window, ON);
				set_authorization_timer(current_window, AUTHORIZATION_EXTENSION_TIME_MS);
				current_window -> authorizationNextState = OVERRIDE;
				break;
			}
			if (!check_authorization_input(current_window) && // Signal can return to offf
				(check_authorization_time_rollover(current_window) || check_authorization_release_request(current_window)))
			{
				clear_authorization_fsm_input_flags(&(current_window -> flags));
				//turn auth override off
				write_authorization(current_window, OFF);
				turn_off_authorization_timer(&(current_window -> flags));
				current_window -> authorizationNextState = RELEASED;
				break;
			}
			if (!check_authorization_time_rollover(current_window) && // Timeout running
				!check_authorization_input(current_window) &&
				!check_authorization_release_request(current_window))
			{
				clear_authorization_fsm_input_flags(&(current_window -> flags));
				//turn auth override on
				write_authorization(current_window, ON);
				current_window -> authorizationNextState = OVERRIDE;
				break;
			}
		}
	}
}

void output_fsm_fire(window_t* current_window)
{
	current_window -> outputState = current_window -> outputNextState; // Updates state
	switch (current_window -> outputState)
	{
		default:
		case ZERO:
		{			
			cache_output_request(current_window);
			set_output(current_window, NONE);
			clear_central_close_in_progress(&(current_window -> flags));
			if (check_output_request(current_window) == NONE) // CACHED
			{
				current_window -> outputNextState = ZERO;
				break;
			}
			switch (check_output_request(current_window)) //cached
			{
				default:
				case NONE:
				{
					current_window -> outputNextState = ZERO;
					return;					
				}
				case UP_PRESSED:
				{
					write_up(current_window, ON);
					break;	
				}
				case DOWN_PRESSED:
				{
					write_down(current_window, ON);
					break;	
				}
				case UP_RELEASED_AUTO:
				{
					write_down(current_window, OFF);
					// Semaphore for not releasing authorization before button releasing sequence is over in up mode
					set_central_close_in_progress(&(current_window -> flags));
					break;	
				}
				case DOWN_RELEASED_AUTO:
				{
					write_up(current_window, OFF);
					break;	
				}
				case UP_RELEASED_STOP:
				{
					write_up(current_window, OFF);
					break;	
				}
				case DOWN_RELEASED_STOP:
				{
					write_down(current_window, OFF);
					break;	
				}
			}
			set_output_timer(current_window, BUTTON_SEQUENCE_DELAY_BETWEEN_STEPS_MS);
			current_window -> outputNextState = FIRST;
			break;		
		}
		case FIRST:
		{
			if (check_output_request(current_window) == NONE) // CACHED
			{
				turn_off_output_timer(&(current_window -> flags));
				current_window -> outputNextState = ZERO;
				break;
			}
			if (check_output_time_rollover(current_window))
			{
				turn_off_output_timer(&(current_window -> flags));
				current_window -> outputNextState = SECOND;
				break;
			}
			else
			{
				current_window -> outputNextState = FIRST;
				break;
			}
		}
		case SECOND:
		{
			if (check_output_request(current_window) == NONE) // CACHED
			{
				current_window -> outputNextState = ZERO;
				break;
			}
			switch (check_output_request(current_window)) //cached
			{
				default:
				case NONE:
				{
					current_window -> outputNextState = ZERO;
					return;
				}
				case UP_PRESSED:
				{
					write_down(current_window, ON);
					break;
				}
				case DOWN_PRESSED:
				{
					write_up(current_window, ON);
					break;
				}
				case UP_RELEASED_AUTO:
				{
					write_up(current_window, OFF);
					break;
				}
				case DOWN_RELEASED_AUTO:
				{
					write_down(current_window, OFF);
					break;
				}
				case UP_RELEASED_STOP:
				{
					write_down(current_window, OFF);
					break;
				}
				case DOWN_RELEASED_STOP:
				{
					write_up(current_window, OFF);
					break;
				}
			}
			set_output_timer(current_window, BUTTON_SEQUENCE_DELAY_BETWEEN_STEPS_MS);
			current_window -> outputNextState = THIRD;
			break;
		}
		case THIRD:
		{
			if (check_output_request(current_window) == NONE) // CACHED
			{
				turn_off_output_timer(&(current_window -> flags));
				current_window -> outputNextState = ZERO;
				break;
			}
			if (check_output_time_rollover(current_window))
			{
				turn_off_output_timer(&(current_window -> flags));				
				current_window -> outputNextState = ZERO;				
				break;
			}
			else
			{
				current_window -> outputNextState = THIRD;
				break;
			}
		}
	}
}

// Window FSM input functions
void clear_window_fsm_input_flags(volatile FLAGS* flags)
{
	//flags -> down_sw = 0;
	//flags -> up_sw = 0;
	//flags -> up_rem_sw = 0;
	//flags -> down_rem_sw = 0;
	//flags -> cclose_in = 0;
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

unsigned char check_mismatch(window_t* current_window)
{
	return (((current_window->flags).up_sw && (current_window->flags).down_rem_sw) ||
			((current_window->flags).up_rem_sw && (current_window->flags).down_sw)); 
}

unsigned char check_mismatch_auto_up(window_t* current_window)
{
	return (((current_window->flags).up_sw && (current_window->flags).down_sw && (current_window->flags).down_rem_sw &&  !(current_window->flags).up_rem_sw) ||
			((current_window->flags).up_rem_sw && (current_window->flags).down_rem_sw && (current_window->flags).down_sw &&  !(current_window->flags).up_sw));
}

unsigned char check_mismatch_auto_down(window_t* current_window)
{
	return (((current_window->flags).up_sw && (current_window->flags).down_sw && !(current_window->flags).down_rem_sw &&  (current_window->flags).up_rem_sw) ||
	((current_window->flags).up_rem_sw && (current_window->flags).down_rem_sw && !(current_window->flags).down_sw &&  (current_window->flags).up_sw));
}

unsigned char check_ot_time_rollover(window_t* current_window)
{
	return (current_window -> flags).ot_timer_rollover;
}

unsigned char check_central_close(window_t* current_window)
{
	return (current_window -> flags).cclose_in;
}

unsigned char check_no_central_close(window_t* current_window)
{
	return !(current_window -> flags).cclose_in;
}

unsigned char check_central_close_over(window_t* current_window)
{
	return (current_window -> flags).cclose_over;
}

// Window FSM output functions
void set_output(window_t* current_window, OUTPUT_REQUEST o)
{
	(current_window -> output) = o;
}

void set_window_OT_timer(window_t* current_window, int time)
{
	current_window -> ot_timer_max_count = time;
	current_window -> ot_timer_counter = 0;
	current_window -> flags.ot_timer_rollover = 0;
	current_window -> flags.ot_timer_enable = ON;
	return;
}

void turn_off_window_OT_timer(volatile FLAGS* flags)
{
	flags -> ot_timer_enable = OFF;
	return;
}

void set_authorization_request_flags(window_t* current_window, unsigned char auth_state)
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

// Output FSM input functions
OUTPUT_REQUEST check_output_request(window_t* current_window)
{
	return current_window -> output_cache;
}

void cache_output_request(window_t* current_window)
{
	current_window -> output_cache = current_window -> output;
}

unsigned char check_output_time_rollover(window_t* current_window)
{
	return (current_window -> flags).output_timer_rollover;
}

// Output FSM output functions
void write_up(window_t* current_window, unsigned char level)
{
	// Automotive line is active low
	// Output stage pulls it low when control line is high
	// A level variable high shall pull the automotive line low
	// Therefore, if (level) GPIO goes high	
	if (level)
	{
		UP_OUT_PORT |= (1 << UP_OUT_BIT_POS);
	}
	else
	{
		UP_OUT_PORT &= ~(1 << UP_OUT_BIT_POS);
	}
}

void write_down(window_t* current_window, unsigned char level)
{
	// Automotive line is active low
	// Output stage pulls it low when control line is high
	// A level variable high shall pull the automotive line low
	// Therefore, if (level) GPIO goes high
	if (level)
	{
		DOWN_OUT_PORT |= (1 << DOWN_OUT_BIT_POS);
	}
	else
	{
		DOWN_OUT_PORT &= ~(1 << DOWN_OUT_BIT_POS);
	}
}

void set_central_close_in_progress(volatile FLAGS* flags)
{
	flags -> cclose_over = 0;
}

void clear_central_close_in_progress(volatile FLAGS* flags)
{
	flags -> cclose_over = 1;
}

void set_output_timer(window_t* current_window, int time)
{
	current_window -> output_timer_max_count = time;
	current_window -> output_timer_counter = 0;
	current_window -> flags.output_timer_rollover = 0;
	current_window -> flags.output_timer_enable = ON;
	return;
}

void turn_off_output_timer(volatile FLAGS* flags)
{
	flags -> output_timer_enable = OFF;
	return;
}

// Authorization FSM input functions
void clear_authorization_fsm_input_flags(volatile FLAGS* flags)
{
	//flags -> authorization_in = 0;
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

// Authorization FSM output functions
void write_authorization(window_t* current_window, unsigned char level)
{
	// Automotive line is active low
	// Output stage pulls it low when control line is high
	// A level variable high shall pull the automotive line low
	// Therefore, if (level) GPIO goes high
	if (level)
	{
		AUTH_OUT_PORT |= (1 << AUTH_OUT_BIT_POS);
	}
	else
	{
		AUTH_OUT_PORT &= ~(1 << AUTH_OUT_BIT_POS);
	}
}

void set_authorization_timer(window_t* current_window, int time)
{
	current_window -> authorization_timer_max_count = time;
	current_window -> authorization_timer_counter = 0;
	current_window -> flags.authorization_timer_rollover = 0;
	current_window -> flags.authorization_timer_enable = ON;
	return;
}

void turn_off_authorization_timer(volatile FLAGS* flags)
{
	flags -> authorization_timer_enable = OFF;
	return;
}

void initDebugLine()
{
#ifdef DEBUG_MODE
	DEBUG_PORT &= ~(1 << DEBUG_BIT_POS);
	DEBUG_DDR |= (1 << DEBUG_BIT_POS);
#endif
}

void setDebugLine()
{
#ifdef DEBUG_MODE
	DEBUG_PORT |= (1 << DEBUG_BIT_POS);
#endif
}

void clearDebugLine()
{
#ifdef DEBUG_MODE
	DEBUG_PORT &= ~(1 << DEBUG_BIT_POS);	
#endif
}

void toggleDebugLine()
{
#ifdef DEBUG_MODE
	DEBUG_PORT ^= (1 << DEBUG_BIT_POS);
#endif
}









