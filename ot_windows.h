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


#define OFF     0
#define ON      1
#define UP      2
#define DOWN    3

typedef struct _FLAGS 
{
    unsigned char up_sw : 1;
    unsigned char down_sw : 1;
	unsigned char up_rem_sw : 1;
	unsigned char down_rem_sw : 1;
	unsigned char cclose_in : 1;
	unsigned char cclose_over : 1;
    unsigned char ot_timer_rollover : 1;
    unsigned char ot_timer_enable : 1;
} FLAGS;

typedef enum _OUTPUT
{
	NONE,
	UP_PRESSED,
	DOWN_PRESSED,
	UP_RELEASED_AUTO,
	DOWN_RELEASED_AUTO,
	UP_RELEASED_STOP,
	DOWN_RELEASED_STOP
} OUTPUT;

typedef struct window 
{
    unsigned char id;
    unsigned char current_state;
    unsigned char next_state;
    volatile int ot_timer_counter;
    int ot_timer_max_count;        
    volatile FLAGS flags;
	OUTPUT output;
} window_t;

void window_fsm_fire(window_t*);
inline void windows_fsm_fire_all(void);

void windows_init(void);
window_t get_window(unsigned char);
window_t* get_window_pointer(unsigned char);
unsigned char set_window_id(unsigned char, unsigned char);

inline void set_OT_timer(window_t*, int);
inline void set_output(window_t*, OUTPUT);

inline void clear_input_flags(volatile FLAGS*);

inline void turn_off(volatile FLAGS*);
inline void turn_off_OT_timer(volatile FLAGS*);

void timer_init(void);
void port_init(void);
void read_port(void);
void set_port(void);
void set_timer_flags(void);
void timer_interrupt(void);

// Input functions
unsigned char check_up_and_no_down(window_t* current_window); //yellow
unsigned char check_down_and_no_up(window_t* current_window); //green
unsigned char check_up_and_down(window_t* current_window); //orange
unsigned char check_all_released(window_t* current_window); // blue
unsigned char check_any_pressed(window_t* current_window); //red
unsigned char check_no_up_and_no_down(window_t* current_window); // gray
unsigned char check_up(window_t* current_window); //pink
unsigned char check_down(window_t* current_window); // same as pink for down
unsigned char check_ot_time_rollover(window_t* current_window);
unsigned char check_central_close(window_t* current_window);
unsigned char check_central_close_over(window_t* current_window);

#ifdef	__cplusplus
}
#endif

#endif	/* OT_WINDOWS_H */

