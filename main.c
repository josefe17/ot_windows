
#include "ot_windows.h"
#include "config.h"
#include <xc.h>


void main (void){
    //Oscillator configurations
    OSCCONbits.IRCF=0b110; //  4MHz, Config bits source
    OSCCONbits.SCS=0b11;   //   Int OSC
    port_init();
    windows_init();
    timer_init();
    GIE=ON;
    while(1){
        read_port();
        set_timer_flags();
        windows_fsm_fire_all();
        set_port();
    }
   
}

void interrupt ISR(void){
    timer_interrupt();
}