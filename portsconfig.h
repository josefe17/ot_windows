/* 
 * File:   portsconfig.h
 * Author: josefe
 *
 * Created on 19 de agosto de 2016, 22:51
 */

#ifndef PORTSCONFIG_H
#define	PORTSCONFIG_H

#ifdef	__cplusplus
extern "C" {
#endif
    

    
#include <xc.h>

    /*I/O ports*/
    //TODO
    
#define TEST_4550
    
#ifdef TEST_4550
    
    #define WINDOW1_SW_UP1          RA0
    #define WINDOW1_SW_UP2          RA2
    #define WINDOW1_SW_DOWN1        RA1
    #define WINDOW1_SW_DOWN2        RA3
    #define WINDOW1_ROLLUP          LATB5
    #define WINDOW1_ROLLDOWN        LATB4
    #define WINDOW1_CURRENT_SENSE   RB3

    
    #define WINDOW1_SW_UP1_TRIS         TRISA0
    #define WINDOW1_SW_UP2_TRIS         TRISA2
    #define WINDOW1_SW_DOWN1_TRIS       TRISA1
    #define WINDOW1_SW_DOWN2_TRIS       TRISA3
    #define WINDOW1_ROLLUP_TRIS         TRISB5
    #define WINDOW1_ROLLDOWN_TRIS       TRISB4
    #define WINDOW1_CURRENT_SENSE_TRIS  TRISB3

    #define WINDOW2_SW_UP1          RA4
    #define WINDOW2_SW_UP2          RA6
    #define WINDOW2_SW_DOWN1        RA5
    #define WINDOW2_SW_DOWN2        RC0
    #define WINDOW2_ROLLUP          LATB2
    #define WINDOW2_ROLLDOWN        LATB1
    #define WINDOW2_CURRENT_SENSE   RB0

    #define WINDOW2_SW_UP1_TRIS         TRISA4
    #define WINDOW2_SW_UP2_TRIS         TRISA6
    #define WINDOW2_SW_DOWN1_TRIS       TRISA5
    #define WINDOW2_SW_DOWN2_TRIS       TRISC0
    #define WINDOW2_ROLLUP_TRIS         TRISB2
    #define WINDOW2_ROLLDOWN_TRIS       TRISB1
    #define WINDOW2_CURRENT_SENSE_TRIS  TRISB0
    
#endif

#ifdef	__cplusplus
}
#endif

#endif	/* PORTSCONFIG_H */

