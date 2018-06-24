

//#define COUNT (BOARD_CPU_CLOCK/1000) //   1 ms
//#define COUNT (BOARD_CPU_CLOCK)      //   1  s
#define COUNT (BOARD_CPU_CLOCK/100)    //  10 ms
//#define COUNT (BOARD_CPU_CLOCK/4)    //  250 ms


//#define _CYCLE_TIME

#include "function.h"


typedef struct FLAG{
	char fft :1 ;
	char read:1 ;
}Flag;
volatile Flag flag = {0,0};

int counter = 0 ; // count 10*500 = 5s

volatile int breath_data[ LENGTH ] = { 0 } ;
volatile int input_breath_data[ RENEW_TIME ] = { 0 } ;

volatile int hp_data[ BL ] = { 0 } ;
volatile int32_t input_heart_data[ RENEW_TIME ] = { 0 } ;
volatile int32_t heart_data[ LENGTH ] = { 0 } ;

volatile uint64_t RTC_time = 0 ;


// FFT
double vReal[ LENGTH ] = { 0 } ;
double vImag[ LENGTH ] = { 0 } ;
// end FFT


volatile uint16_t heart_rate = 0 , breath_rate = 0 ;

uint8_t _string[100] = { 0 };


uint16_t led_toggle_val = 0b10101010;
// led_write(led_toggle_val, BOARD_LED_MASK);
// led_toggle_val = ~led_toggle_val;

// initial Hardware
AD7991_DEFINE(ad7991, DEV_ID_0, ADC_I2C_SLAVE_ADDRESS);
HM1X_DEFINE(hm1x, HM_1X_UART_ID);
// end initial Hardware



#ifdef _CYCLE_TIME
uint64_t time3 = 0 ;
uint64_t time4 = 0 ;
#endif

static int64_t x = 0 ;

int main()
{
	uint16_t led_toggle_val = 0b10101010;
	uint32_t baudrate = UART_BAUDRATE_38400;

	// initial Hardware
	ad7991_adc_init(ad7991);
	hm1x_init(hm1x, baudrate);
	// end initial Hardware


	// initial High pass filter data

	// end initial High pass filter data



	// timer ISR begin
	timer_stop(TIMER_1);
	int_disable(INTNO_TIMER1);
	int_handler_install(INTNO_TIMER1, timer1_isr);
	int_pri_set(INTNO_TIMER1, INT_PRI_MIN);
	EMBARC_PRINTF("\r\n/******** TEST MODE START ********/\r\n\r\n");
	int_enable(INTNO_TIMER1);
	timer_start(TIMER_1, TIMER_CTRL_IE | TIMER_CTRL_NH, COUNT);
	// end timer ISR begin


	while (x<29736)
	{

		if( flag.fft == 1 )
		{
#ifdef _CYCLE_TIME
			time3 = board_get_cur_us();
			EMBARC_PRINTF("start FFT\r\n");
#endif


			// shift data
			//EMBARC_PRINTF("heart_data : ");
			for (int i = 0; i < (LENGTH - RENEW_TIME) ; ++i)
			{
				// breath_data[i] = breath_data[i + 1];
				heart_data[i] = heart_data[i + RENEW_TIME];
			//	EMBARC_PRINTF(" %d ,",heart_data[i]);
			}
			for (int i = (LENGTH - RENEW_TIME); i < LENGTH ; ++i)
			{
				// breath_data[i] = input_breath_data[ i - LENGTH - RENEW_TIME ] ;
				heart_data[i]  =  input_heart_data[ i - (LENGTH - RENEW_TIME) ] ;
			//	EMBARC_PRINTF(" %d ,",heart_data[i]);
			}

			//EMBARC_PRINTF("\r\n");


			// calculate heart rate

			//FFT
			for (int i = 0; i < LENGTH; ++i)
			{
				vReal[i] = heart_data[i] ;
				vImag[i] = 0 ;
			}
			FFT();//filter_data => FFT_data
			//end FFT


			// find heart rate pulse from vReal
			heart_rate = (uint16_t)Heart( );//FFT_data
			EMBARC_PRINTF("heart = %d \r\n",heart_rate);
			// end find heart rate pulse from vReal


			flag.fft = 0 ;
			led_write(led_toggle_val, BOARD_LED_MASK);
			led_toggle_val = ~led_toggle_val;

#ifdef _CYCLE_TIME
			time3 = board_get_cur_us() - time3 ;
			EMBARC_PRINTF("FFT TIME = %u us \r\n", time3 );
			EMBARC_PRINTF("End FFT\r\n\r\n");
#endif
		}
	}

	return 0;
}


// each 10 ms
static void timer1_isr(void *ptr){


	timer_int_clear(TIMER_1);
	RTC_time ++ ;


//	time4 = board_get_cur_us() - time4 ;
//	EMBARC_PRINTF("ISR TIME = %5u us \r\n", time4 );
//	time4 = board_get_cur_us();


#ifdef _CYCLE_TIME
	time4 = board_get_cur_us();
#endif


	input_breath_data[ counter ] = get_adc( 0 ) ;


	// High pass
	input_heart_data[ counter ] = 0 ;
	for (int j = 0; j < BL ; j++) {
		input_heart_data[ counter ] += (int32_t)hp_data[ j ] * (int32_t)B[j] ;
		if ( j == BL - 1 )
			hp_data[ j ] = input_breath_data[ counter ] ;
		else
			hp_data[j] = hp_data[j + 1] ;
	}
	input_heart_data[ counter ] /= 1024 ;
	// END High pass



	// breath
	// calculate breath rate
	breath_rate = Breath( input_breath_data[ counter ] );//data
	// end breath



	sprintf( _string , "%3d , %3d , %4d , %d\r\n", heart_rate , breath_rate , input_breath_data[counter] , input_heart_data[counter]);
	EMBARC_PRINTF( "%s",_string );
	hm1x_write(hm1x, _string, strlen(_string));


	counter ++ ;

	if( counter >= RENEW_TIME ){
		flag.fft = 1 ;
		counter = 0 ;
	}

//	EMBARC_PRINTF(" ISR counter = %3d , flag.fft = %d \r\n",counter,flag.fft);


#ifdef _CYCLE_TIME
	time4 = board_get_cur_us() - time4 ;
	EMBARC_PRINTF("running TIME = %u us \r\n", (board_get_cur_us()-time4) );
#endif


}

uint16_t get_adc( uint8_t channel ) {
	float buffer[4] = { 0 };
	uint16_t data[4] = { 0 };

	x++ ;
	return data_chang[x] ;

	ad7991_adc_read(ad7991, buffer);

	data[ channel ] = (uint16_t)buffer[ channel ] ;
	return data[ channel ] ;
}


