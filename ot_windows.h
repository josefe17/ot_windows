/* 
 * File:   ot_windows.h
 * Author: josefe
 *
 * Created on 19 de agosto de 2016, 13:06
 */

#ifndef OT_WINDOWS_H
#define	OT_WINDOWS_H

#ifdef	__cplusplus
extern "C" {
#endif
    
#ifndef inline
#define inline
#endif
    
#define NUM_WINDOWS 2
#define RIGHT   'r'
#define LEFT    'l'

#define BUTTON_SEQUENCE_DELAY_BETWEEN_STEPS_MS 15

#define OFF     0
#define ON      1
#define UP      2
#define DOWN    3

typedef enum _OUTPUT_REQUEST
{
	NONE,
	UP_PRESSED,
	DOWN_PRESSED,
	UP_RELEASED_AUTO,
	DOWN_RELEASED_AUTO,
	UP_RELEASED_STOP,
	DOWN_RELEASED_STOP
} OUTPUT_REQUEST;

/*FSM STATUS*/
typedef enum _WINDOW_FSM_STATE
{
	IDLE,
	BLOCKED,
	AUTO_DOWN,
	AUTO_UP,
	MAN_DOWN,
	MAN_UP,
	CENTRAL_CLOSE,
	AUTHORIZATION_OFF
} WINDOW_FSM_STATE;

typedef enum _AUTHORIZATION_FSM_STATE
{
	OFF,
	ARMED,
	OVERRIDE
} AUTHORIZATION_FSM_STATE;

typedef enum _BUTTON_SEQUENCE_STATE
{		
	UP_PRESSED_UP_LOW_1,
	UP_PRESSED_UP_LOW_DELAY_2,
	UP_PRESSED_DOWN_LOW_3,
	UP_PRESSED_DOWN_LOW_DELAY_4,
	
	DOWN_PRESSED_DOWN_LOW_1,
	DOWN_PRESSED_DOWN_LOW_DELAY_2,
	DOWN_PRESSED_UP_LOW_3,
	DOWN_PRESSED_UP_LOW_DELAY_4,
	
	UP_RELEASED_AUTO_DOWN_HIGH_1,
	UP_RELEASED_AUTO_DOWN_HIGH_DELAY_2,
	UP_RELEASED_AUTO_UP_HIGH_3,
	UP_RELEASED_AUTO_UP_HIGH_DELAY_4,
	
	DOWN_RELEASED_AUTO_UP_HIGH_1,
	DOWN_RELEASED_AUTO_UP_HIGH_DELAY_2,
	DOWN_RELEASED_AUTO_DOWN_HIGH_3,
	DOWN_RELEASED_AUTO_DOWN_HIGH_DELAY_4,
	
	UP_RELEASED_STOP_UP_HIGH_1,
	UP_RELEASED_STOP_UP_HIGH_DELAY_2,
	UP_RELEASED_STOP_DOWN_HIGH_3,
	UP_RELEASED_STOP_DOWN_HIGH_DELAY_4,

	DOWN_RELEASED_STOP_DOWN_HIGH_1,
	DOWN_RELEASED_STOP_DOWN_HIGH_DELAY_2,
	DOWN_RELEASED_STOP_UP_HIGH_3,
	DOWN_RELEASED_STOP_UP_HIGH_DELAY_4
} BUTTON_SEQUENCE_STATE;

typedef enum _OUTPUT_FSM_STATE
{
	IDLE,
	FIRST, // Delay
	SECOND, // Second action
	THIRD // Second delay
} OUTPUT_FSM_STATE;

typedef struct _FLAGS 
{
    unsigned char up_sw : 1; 
    unsigned char down_sw : 1;
	unsigned char up_rem_sw : 1;
	unsigned char down_rem_sw : 1;
	unsigned char cclose_in : 1;
	unsigned char cclose_over : 1;	
	unsigned char authorization_in: 1;
	unsigned char authorization_on: 1;
	unsigned char authorization_off: 1;
    unsigned char ot_timer_rollover : 1;
    unsigned char ot_timer_enable : 1;
	unsigned char output_timer_rollover: 1;
	unsigned char output_timer_enable: 1;
	unsigned char authorization_timer_rollover : 1;
	unsigned char authorization_timer_enable : 1;
} FLAGS;


typedef struct window 
{
    unsigned char id;
    WINDOW_FSM_STATE current_state;
    WINDOW_FSM_STATE next_state;
    volatile int ot_timer_counter;
    int ot_timer_max_count;  
	OUTPUT_REQUEST output;
	OUTPUT_REQUEST output_cache;
	OUTPUT_FSM_STATE outputState;
	OUTPUT_FSM_STATE outputNextState;
	volatile int output_timer_counter;
	int output_timer_max_count;
	AUTHORIZATION_FSM_STATE authorizationState;
	AUTHORIZATION_FSM_STATE authorizationNextState;
	volatile int authorization_timer_counter;
	int authorization_timer_max_count;
    volatile FLAGS flags;

} window_t;

inline void windows_fsm_fire_all(void);

void window_fsm_fire(window_t*);
void output_fsm_fire(window_t*);
void authorization_fsm_fire(window_t*);

void windows_init(void);
window_t get_window(unsigned char);
window_t* get_window_pointer(unsigned char);
unsigned char set_window_id(unsigned char, unsigned char);

void timer_init(void);
void port_init(void);
void read_port(void);
void set_port(void);
void set_timer_flags(void);
volatile void timer_interrupt(void);

// Window FSM input functions
inline void clear_window_fsm_input_flags(volatile FLAGS*);
unsigned char check_up_and_no_down(window_t*); //yellow
unsigned char check_down_and_no_up(window_t*); //green
unsigned char check_up_and_down(window_t*); //orange
unsigned char check_all_released(window_t*); // blue
unsigned char check_any_pressed(window_t*); //red
unsigned char check_no_up_and_no_down(window_t*); // gray
unsigned char check_up(window_t*); //pink
unsigned char check_down(window_t*); // same as pink for down
unsigned char check_ot_time_rollover(window_t*);
unsigned char check_central_close(window_t*);
unsigned char check_central_close_over(window_t*);

// Window FSM output functions
inline void set_output(window_t*, OUTPUT_REQUEST);
inline void set_window_OT_timer(window_t*, int);
inline void turn_off_window_OT_timer(volatile FLAGS*);
void set_authorization_request_flags(window_t*, unsigned char);

// Output FSM input functions
OUTPUT_REQUEST check_output_request(window_t*);
void cache_output_request(window_t*);
unsigned char check_output_time_rollover(window_t*);

// Output FSM output functions
// Use inline void set_output(window_t*, OUTPUT_REQUEST) to clear input flags
inline void write_up(window_t*, unsigned char);
inline void write_down(window_t*, unsigned char);
inline void set_central_close_in_progress(volatile FLAGS*);
inline void clear_central_close_in_progress(volatile FLAGS*);
inline void set_output_timer(window_t*, int);
inline void turn_off_output_timer(volatile FLAGS*);

//Authorization FSM input functions
inline void clear_authorization_fsm_input_flags(volatile FLAGS*);
unsigned char check_authorization_input(window_t*);
unsigned char check_authorization_override_request(window_t*);
unsigned char check_authorization_release_request(window_t*);
unsigned char check_authorization_time_rollover(window_t*);

// Authorization FSM output functions
inline void write_authorization(window_t*, unsigned char);
inline void set_authorization_timer(window_t*, int);
inline void turn_off_authorization_timer(volatile FLAGS*);

#ifdef	__cplusplus
}
#endif

#endif	/* OT_WINDOWS_H */

