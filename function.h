
#ifndef _FUNCTION_H
#define _FUNCTION_H


#include <stdio.h>
#include <stdlib.h>

#include "embARC.h"
#include "embARC_debug.h"
#include "board.h"
#include "dev_uart.h"
#include "hm1x.h"
#include "dev_iic.h"
#include "ad7991.h"
#include <math.h>

//#include "data.h"
#include "FFT.h"

extern int data_chang [29317] ;


/*
 * highpass FIR (Equiripple)
 * fs = 100 fpass = 1 fstop = 0.5
 */

/*
 * highpass FIR (Equiripple)
 * fs = 100 fpass = .8 fstop = 0.3
 */
#define BL 101
extern const int16_t B[BL] ;

extern int counter ; // count 10*500 = 5s

#define RENEW_TIME 600
#define LENGTH 4096
extern volatile int breath_data[ LENGTH ] ;
extern volatile int input_breath_data[ RENEW_TIME ] ;

extern volatile int hp_data[ BL ] ;
extern volatile int32_t input_heart_data[ RENEW_TIME ] ;
extern volatile int32_t heart_data[ LENGTH ] ;

extern double vReal[ LENGTH ] ;
extern double vImag[ LENGTH ] ;

extern uint8_t _string[100] ;

extern volatile uint64_t RTC_time ;

//void ISR();
static void timer1_isr(void *ptr);

int Breath( int data );
void FFT() ;
double Heart();
uint16_t get_adc( uint8_t channel );

#endif
