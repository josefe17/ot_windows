/*
 * ports.h
 *
 * Created: 09/05/2023 15:24:46
 *  Author: josefe
 */ 

#include <avr/io.h>

#ifndef PORTS_H_
#define PORTS_H_

#define UP_OUT_PORT PORTD
#define UP_OUT_DDR DDRD
#define UP_OUT_BIT_POS 0

#define DOWN_OUT_PORT PORTD
#define DOWN_OUT_DDR DDRD
#define DOWN_OUT_BIT_POS 1

#define AUTH_OUT_PORT PORTD
#define AUTH_OUT_DDR DDRD
#define AUTH_OUT_BIT_POS 2

#define AUTH_IN_PORT PORTD
#define AUTH_IN_PIN PIND
#define AUTH_IN_DDR DDRD
#define AUTH_IN_BIT_POS 3

#define CCLOSE_IN_PORT PORTD
#define CCLOSE_IN_PIN PIND
#define CCLOSE_IN_DDR DDRD
#define CCLOSE_IN_BIT_POS 4

#define UP_IN_PORT PORTD
#define UP_IN_PIN PIND
#define UP_IN_DDR DDRD
#define UP_IN_BIT_POS 5

#define DOWN_IN_PORT PORTD
#define DOWN_IN_PIN PIND
#define DOWN_IN_DDR DDRD
#define DOWN_IN_BIT_POS 6

#define UP_REM_PORT PORTB
#define UP_REM_PIN PINB
#define UP_REM_DDR DDRB
#define UP_REM_BIT_POS 0

#define DOWN_REM_PORT PORTB
#define DOWN_REM_PIN PINB
#define DOWN_REM_DDR DDRB
#define DOWN_REM_BIT_POS 1

#define DEBUG_PORT PORTB
#define DEBUG_DDR DDRB
#define DEBUG_BIT_POS 2

#endif /* PORTS_H_ */