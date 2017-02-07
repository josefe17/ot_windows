/* 
 * File:   tmrconfig.h
 * Author: josefe
 *
 * Created on 19 de agosto de 2016, 16:37
 */

#ifndef TMRCONFIG_H
#define	TMRCONFIG_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <xc.h>

#define TIMEOUT_UP      4700
#define TIMEOUT_DOWN    4250
#define OT_TIMER_COUNT  250
#define CS_TIMER_COUNT  100

#define TIMER_PR        PR2
#define TIMER_TMR       TMR2
#define TIMER_TCON      T2CON
#define TIMER_IF        TMR2IF
#define TIMER_IE        TMR2IE

#define TIMER_TCON_LOAD 0b00000101; // Prescaler 1/4, no postcaler => Tbit 4us
#define TIMER_PR_LOAD   250; // Tint = 250*4us = 1 ms

#ifdef	__cplusplus
}
#endif

#endif	/* TMRCONFIG_H */

