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
    
#include "tmrconfig.h"
#include "portsconfig.h"

#define NUM_WINDOWS 2
#define RIGHT   'r'
#define LEFT    'l'


#define OFF     0
#define ON      1
#define UP      2
#define DOWN    3

    typedef struct _FLAGS {
        unsigned char up_sw : 1;
        unsigned char down_sw : 1;
        unsigned char output_up : 1;
        unsigned char output_down : 1;
        unsigned char current_sense : 1;
        unsigned char current_sense_timer_rollover :1;
        unsigned char ot_timer_rollover : 1;
        unsigned char timeout : 1;
        unsigned char current_sense_enable : 1;
        unsigned char current_sense_timer_enable :1;
        unsigned char ot_timer_enable : 1;
        unsigned char timeout_enable : 1;
    } FLAGS;

    typedef struct window {
        unsigned char id;
        unsigned char current_state;
        unsigned char next_state;
        volatile int ot_timer_counter;
        int ot_timer_max_count;
        volatile int timeout_timer_counter;
        int timeout_timer_max_count;
        volatile int current_sense_timer_counter;
        int current_sense_timer_max_count;
        volatile FLAGS flags;
    } window_t;

void window_fsm_fire(window_t*);
inline void windows_fsm_fire_all(void);

void windows_init(void);
window_t get_window(unsigned char);
window_t* get_window_pointer(unsigned char);
unsigned char set_window_id(unsigned char, unsigned char);

inline void set_output(volatile FLAGS*, unsigned char);
inline void set_comparators(volatile FLAGS*, unsigned char);
inline void set_safety_timer(window_t*, int);
inline void set_OT_timer(window_t*, int);
inline void set_CS_timer(window_t*, int);

inline void clear_safety_flags(volatile FLAGS*);
inline void clear_input_flags(volatile FLAGS*);
inline void clear_CS_flags(volatile FLAGS*);

inline void turn_off(volatile FLAGS*);
inline void turn_off_OT_timer(volatile FLAGS*);
inline void turn_off_CS_timer(volatile FLAGS*);

void timer_init(void);
void port_init(void);
void read_port(void);
void set_port(void);
void set_timer_flags(void);
void timer_interrupt(void);


#ifdef	__cplusplus
}
#endif

#endif	/* OT_WINDOWS_H */

