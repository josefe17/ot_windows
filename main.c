
#define F_CPU 16000000UL

#include "ot_windows.h"

window_t myWindow;

int main (void)
{
	//TODO
    //Oscillator configurations	  
    ot_window_init(&myWindow);
    sei();
    while(1)
	{
		ot_window_run(&myWindow);
    }
	return 0;   
}

ISR(TIMER0_COMPA_vect)
{
    timer_interrupt();
}