/*
 * timer2.c
 *
 * Created: 04-Mar-2018 
 * Author: Alsayed Alsisi
 * Contact Details:
  - (+2)01066769510
  - alsayed.alsisi@gmail.com
 */ 

/*----------------------------------------------------------------
--------------------- File Inclusions ----------------------------
----------------------------------------------------------------*/
#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include "timer2.h"


/*----------------------------------------------------------------
--------------------- Private Data Types --------------------------
----------------------------------------------------------------*/

typedef enum{
	TIMER2_NORMAL_MODE=0,
	TIMER2_PHASE_CORRECT_PWM_MODE,
	TIMER2_CTC_MODE,
	TIMER2_FAST_PWM_MODE
}timer_mode_t;


typedef enum{
	TIMER2_NO_CLK=0,						//No clock source (Timer/Counter stopped).
	TIMER2_NO_PRESCALAR,					//clk / (No prescaling)
	TIMER2_PRESCALAR8,						//clk / 8
	TIMER2_PRESCALAR32,						//clk / 32
	TIMER2_PRESCALAR64,						//clk / 64
	TIMER2_PRESCALAR128,					//clk / 128
	TIMER2_PRESCALAR256,					//clk / 256
	TIMER2_PRESCALAR1024,					//clk / 1024
	TIMER2_EXTERNAL_CLOCK_FALLING_EDGE,		//
	TIMER2_EXTERNAL_CLOCK_RISING_EDGE		//
}timer_prescalar_t;


/*----------------------------------------------------------------
--------------------- Private Function Prototypes ----------------
----------------------------------------------------------------*/
static void timer2_mode_set(timer_mode_t timer_mode);
static void timer2_prescalar_set(timer_prescalar_t timer_prescalar);
static void timer2_interrupt_enable(timer_mode_t timer_mode);
static void timer2_interrupt_disable(void);
static void timer2_TCNT2_load(uint8_t value_to_load);
static void timer2_OCR2_load( uint8_t value_to_load);
static void timer2_create_delay_us(uint8_t value_to_load_ocr2, timer_prescalar_t timer_prescalar);

/*----------------------------------------------------------------
--------------------- Public Function Definitions ----------------
----------------------------------------------------------------*/

// Initializing the timer to work in PWM mode:
void timer2_pwm_init(void)
{
	//Setting the timer to work in Fast PWM Mode:
	timer2_mode_set(TIMER2_FAST_PWM_MODE);
	
	//set the desired initial duty cycle:
	timer2_pwm_duty_cycle_set(100);		// 100%
	
}

// To start the PWM signal:
void timer2_pwm_start(void)
{
	timer2_prescalar_set(TIMER2_NO_PRESCALAR);
}

//To change the duty cycle as required. And note that 0% duty cycle means no output on the pwm OC0 pin.
void timer2_pwm_duty_cycle_set(uint8_t duty_cycle_percentage)
{
	if (duty_cycle_percentage == 0)
	{ OCR2 = 0; }
	else
	{ OCR2 = ( ( (duty_cycle_percentage * 255) / 100) - 1 ); }
}

/*----------------------------------------------------------------
--------------------- Private Function Definitions ----------------
----------------------------------------------------------------*/

static void timer2_mode_set(timer_mode_t timer_mode)
{
	switch (timer_mode)
	{
		case TIMER2_NORMAL_MODE:
		TCCR2 &= ~((1<<WGM21)|(1<<WGM20));
		break;
		
		case TIMER2_CTC_MODE:
		TCCR2 |= (1<<WGM21);
		TCCR2 &= ~(1<<WGM20);
		break;
		
		case TIMER2_FAST_PWM_MODE:
		TCCR2 |= ((1<<WGM21)|(1<<WGM20));
		TCCR2 |= (1<<COM21);				//Non-inverted fast PWM mode.
		TCCR2 &= ~(1<<COM20);
		DDRD  |= (1<<7);					//setting OC2 as output to output the PWM signal
		break;
		
		default:
		break;
	}
	
}



static void timer2_prescalar_set(timer_prescalar_t timer_prescalar)
{
	
	switch (timer_prescalar)
	{
		case TIMER2_NO_CLK:
		TCCR2 &= ~((1<<CS22)|(1<<CS21)|(1<<CS20));
		break;
		
		case TIMER2_NO_PRESCALAR:
		TCCR2 |= (1<<CS20);
		TCCR2 &= ~((1<<CS22)|(1<<CS21));
		break;
		
		case TIMER2_PRESCALAR8:
		TCCR2 |= (1<<CS21);
		TCCR2 &= ~((1<<CS22)|(1<<CS20));
		break;
		
		case TIMER2_PRESCALAR32:
		TCCR2 |=  ((1<<CS21)|(1<<CS20));
		TCCR2 &= ~(1<<CS22);
		break;
		
		case TIMER2_PRESCALAR64:
		TCCR2 |= (1<<CS22);
		TCCR2 &=  ~((1<<CS21)|(1<<CS20));
		break;
		
		case TIMER2_PRESCALAR128:
		TCCR2 |=  ((1<<CS22)|(1<<CS20));
		TCCR2 &= ~(1<<CS21);
		break;
		
		case TIMER2_PRESCALAR256:
		TCCR2 |= ((1<<CS22)|(1<<CS21));
		TCCR2 = ~(1<<CS20);
		break;
		
		case TIMER2_PRESCALAR1024:
		TCCR2 |=  ((1<<CS22)|(1<<CS21)|(1<<CS20));
		break;
		
		default:
		break;
	}
	
}



static void timer2_interrupt_enable(timer_mode_t timer_mode)
{
	switch(timer_mode)
	{
		case TIMER2_NORMAL_MODE:
		TIMSK |= (1<<TOIE2);
		sei();
		break;
		
		case TIMER2_CTC_MODE:
		TIMSK |= (1<<OCIE2);
		sei();
		break;
		
		default:
		break;
	}
}

static void timer2_interrupt_disable(void)
{
	TIMSK &= ~((1<<TOIE2)|(1<<OCIE2));
}


static void timer2_TCNT2_load(uint8_t value_to_load)
{
	TCNT2  = value_to_load;
}

static void timer2_OCR2_load( uint8_t value_to_load)
{
	OCR2 = value_to_load;
}

static void timer2_create_delay_us(uint8_t value_to_load_ocr2, timer_prescalar_t timer_prescalar)
{
	
	TIMSK &= ~((1<<TOIE2)|(1<<OCIE2));		//disable the timer interrupts.
	TCNT2 = 0;								//to make sure you are starting clean.
	OCR2 = value_to_load_ocr2;				//loading ocr0 register with the desired value.
	switch (timer_prescalar)				//setting the timer clock prescalar.
	{
		case TIMER2_NO_CLK:
		TCCR2 &= ~((1<<CS22)|(1<<CS21)|(1<<CS20));
		break;
		
		case TIMER2_NO_PRESCALAR:
		TCCR2 |= (1<<CS20);
		TCCR2 &= ~((1<<CS22)|(1<<CS21));
		break;
		
		case TIMER2_PRESCALAR8:
		TCCR2 |= (1<<CS21);
		TCCR2 &= ~((1<<CS22)|(1<<CS20));
		break;
		
		case TIMER2_PRESCALAR32:
		TCCR2 |=  ((1<<CS21)|(1<<CS20));
		TCCR2 &= ~(1<<CS22);
		break;
		
		case TIMER2_PRESCALAR64:
		TCCR2 |= (1<<CS22);
		TCCR2 &=  ~((1<<CS21)|(1<<CS20));
		break;
		
		case TIMER2_PRESCALAR128:
		TCCR2 |=  ((1<<CS22)|(1<<CS20));
		TCCR2 &= ~(1<<CS21);
		break;
		
		case TIMER2_PRESCALAR256:
		TCCR2 |= ((1<<CS22)|(1<<CS21));
		TCCR2 = ~(1<<CS20);
		break;
		
		case TIMER2_PRESCALAR1024:
		TCCR2 |=  ((1<<CS22)|(1<<CS21)|(1<<CS20));
		break;
		
		default:
		break;
	}
	while((TIFR & (1<<OCF2)) == 0);										 //check compare match flag.
	TCCR2 = 0x00;														 //stop timer.
	TIFR |= (1<<OCF2);													 //clear flag.
	
}



/*----------------------------------------------------------------
--------------------- End of File --------------------------------
----------------------------------------------------------------*/

