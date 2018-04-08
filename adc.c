/*
 * adc.c
 *
 * Created: 28-Jan-18 10:17:20 PM
 *  Author: Alsayed
 */ 



/*----------------------------------------------------------------
--------------------- File Inclusions ----------------------------
----------------------------------------------------------------*/
#include "adc.h"
#include <stdint.h>
#include <avr/interrupt.h>

/*----------------------------------------------------------------
--------------------- Private Data Types --------------------------
----------------------------------------------------------------*/
typedef enum
{
	ADC_EXTERNAL_AREF=0,		// The AREF pin should be connected to +5v.
	ADC_AVCC_AREF,			// AVCC with external capacitor at AREF pin.
	ADC_INTERNAL_2_56_AREF		// Internal 2.56V Voltage Reference with external capacitor at AREF pin
} reference_voltage_t;

typedef enum{
	ADC_PRESCALAR_2=0,		// clk/2
	ADC_PRESCALAR_4,		// clk/4
	ADC_PRESCALAR_8,		// clk/8
	ADC_PRESCALAR_16,		// clk/16
	ADC_PRESCALAR_32,	        // clk/32
	ADC_PRESCALAR_64,	        // clk/64
	ADC_PRESCALAR_128               // clk/128
}adc_prescalar_t;

typedef enum{
	ADC_FREE_RUNNING_MODE=0,
	ADC_ANALOG_COMPARATOR,
	ADC_EXTERNAL_INTERRUPT_REQUEST_0,
	ADC_TIMER_COUNTER_0_COMPARE_MATCH,
	ADC_TIMER_COUNTER_0_OVERFLOW,
	ADC_TIMER_COUNTER_1_COMPARE_MATCH_B,
	ADC_TIMER_COUNTER_1_OVERFLOW,
	ADC_TIMER_COUNTER_1_CAPTURE_EVENT
}adc_auto_triggering_source_t;





/*----------------------------------------------------------------
--------------------- Private Functions Prototypes ---------------
----------------------------------------------------------------*/
   static void adc_enable(void);
   static void adc_disable(void);
   static void adc_reference_voltage(reference_voltage_t reference_voltage);
   static void adc_set_clock_prescalar(adc_prescalar_t adc_prescalar);
   static uint16_t adc_read_adc_register();
   static void adc_start_conversion(void);
   static void adc_wait_conversion_complete(void);
   static void adc_select_channel(adc_channel_t adc_channel);
   static void adc_enable_interrupts(void);
   static void adc_disable_interrupts(void);
   static void adc_enable_auto_triggerig(void);
   static void adc_disable_auto_triggerig(void);
   static void adc_select_auto_triggering_source(adc_auto_triggering_source_t adc_auto_triggering_source);

/*----------------------------------------------------------------
--------------------- Public Function Definitions ----------------
----------------------------------------------------------------*/
void adc_init(void)
{
	adc_enable();
	adc_reference_voltage(ADC_AVCC_AREF);
	adc_set_clock_prescalar(ADC_PRESCALAR_2);
}


//The following function is used to read an ADC channel using polling technique.
uint16_t adc_read_channel(adc_channel_t adc_channel)
 {
	 uint16_t adc_register_value;
	 adc_disable_interrupts();
	 adc_select_channel(adc_channel); 
	 adc_start_conversion();
	 adc_wait_conversion_complete();
	 adc_register_value = adc_read_adc_register();
	 return adc_register_value;
 }
 

/*----------------------------------------------------------------
--------------------- Private Function Definitions ----------------
----------------------------------------------------------------*/
 static void adc_enable(void)
 {
	ADCSRA |= (1 << ADEN);
 }

 static void adc_disable(void)
 {
   ADCSRA &= ~(1 << ADEN);
 }
 
//The following fn determines the division factor between the XTAL frequency and the input clock to the ADC.
static void adc_reference_voltage(reference_voltage_t reference_voltage)
 {
	
	switch(reference_voltage)
	{
		case ADC_EXTERNAL_AREF:
		ADMUX &= ~((1<<REFS1)|(1<<REFS0));
		break;
		
		case ADC_AVCC_AREF:
		ADMUX &= ~(1<<REFS1);
		ADMUX |= (1<<REFS0);
		break;
		
		case ADC_INTERNAL_2_56_AREF:
		ADMUX |= ((1<<REFS1)|(1<<REFS0));
		break;
		
		default:
		break;
	}
}

static void adc_set_clock_prescalar(adc_prescalar_t adc_prescalar)
{
	switch(adc_prescalar)
	{
		case ADC_PRESCALAR_2:				                       // clk/2
		ADCSRA &= ~((1 << ADPS2) |(1 << ADPS1) | (1 << ADPS0));
		break;
		
		case ADC_PRESCALAR_4:				                       // clk/4
		ADCSRA |= (1 << ADPS1);
		ADCSRA &= ~((1 << ADPS2) | (1 << ADPS0));
		break;
		
		case ADC_PRESCALAR_8:										// clk/8
		ADCSRA |= ((1 << ADPS1) | (1 << ADPS0));
		ADCSRA &= ~(1 << ADPS2);
		break;
		
		case ADC_PRESCALAR_16:										// clk/16
		ADCSRA |= (1 << ADPS2);
		ADCSRA &= ~((1 << ADPS1) | (1 << ADPS0));
		break;
		
		case ADC_PRESCALAR_32:										// clk/32
		ADCSRA |= ((1 << ADPS2) | (1 << ADPS0));
		ADCSRA &= ~(1 << ADPS1);
		break;
		
		case ADC_PRESCALAR_64:										// clk/64
		ADCSRA |= ((1 << ADPS2) | (1 << ADPS1));
		ADCSRA &= ~(1 << ADPS0);
		break;
		
		case ADC_PRESCALAR_128:									// clk/128
		ADCSRA |= ((1 << ADPS2) |(1 << ADPS1) | (1 << ADPS0));
		break;
		
		default:
		break;
	}
}

 static uint16_t adc_read_adc_register()
 {
	 return ADC;
 }
 
static void adc_start_conversion(void)
 {
	 ADCSRA |= (1 << ADSC); 
 }
 
static void adc_wait_conversion_complete(void)
 {
	 while( (ADCSRA & (1 << ADIF)) == 0);
	 ADCSRA |= (1 << ADIF); //Clear the flag. Note that in AVR MCUs, you clear the interrupt flags by writing 1 to them.
 }

 static void adc_select_channel(adc_channel_t adc_channel)
 {
	 
		 switch(adc_channel)
		 {
			  case ADC_CHANNEL_0:
			  ADMUX = ((ADMUX & ~((1<<MUX4)|(1<<MUX3)|(1<<MUX2)|(1<<MUX1)|(1<<MUX0))) | (((1<<MUX4)|(1<<MUX3)|(1<<MUX2)|(1<<MUX1)|(1<<MUX0)) & 0b00000000));
			  break;
			  
			  case ADC_CHANNEL_1:
			  ADMUX = ((ADMUX & ~((1<<MUX4)|(1<<MUX3)|(1<<MUX2)|(1<<MUX1)|(1<<MUX0))) | (((1<<MUX4)|(1<<MUX3)|(1<<MUX2)|(1<<MUX1)|(1<<MUX0)) & 0b00000001));
			  break;
			  
			  case ADC_CHANNEL_2:
			  ADMUX = ((ADMUX & ~((1<<MUX4)|(1<<MUX3)|(1<<MUX2)|(1<<MUX1)|(1<<MUX0))) | (((1<<MUX4)|(1<<MUX3)|(1<<MUX2)|(1<<MUX1)|(1<<MUX0)) & 0b00000010));
			  break;
			  
			  case ADC_CHANNEL_3:
			  ADMUX = ((ADMUX & ~((1<<MUX4)|(1<<MUX3)|(1<<MUX2)|(1<<MUX1)|(1<<MUX0))) | (((1<<MUX4)|(1<<MUX3)|(1<<MUX2)|(1<<MUX1)|(1<<MUX0)) & 0b00000011));
			  break;
			  
			  case ADC_CHANNEL_4:
			  ADMUX = ((ADMUX & ~((1<<MUX4)|(1<<MUX3)|(1<<MUX2)|(1<<MUX1)|(1<<MUX0))) | (((1<<MUX4)|(1<<MUX3)|(1<<MUX2)|(1<<MUX1)|(1<<MUX0)) & 0b00000100));
			  break;
			  
			  case ADC_CHANNEL_5:
			  ADMUX = ((ADMUX & ~((1<<MUX4)|(1<<MUX3)|(1<<MUX2)|(1<<MUX1)|(1<<MUX0))) | (((1<<MUX4)|(1<<MUX3)|(1<<MUX2)|(1<<MUX1)|(1<<MUX0)) & 0b00000101));
			  break;
			  
			  case ADC_CHANNEL_6:
			  ADMUX = ((ADMUX & ~((1<<MUX4)|(1<<MUX3)|(1<<MUX2)|(1<<MUX1)|(1<<MUX0))) | (((1<<MUX4)|(1<<MUX3)|(1<<MUX2)|(1<<MUX1)|(1<<MUX0)) & 0b00000110));
			  break;
			  
			  case ADC_CHANNEL_7:
			  ADMUX = ((ADMUX & ~((1<<MUX4)|(1<<MUX3)|(1<<MUX2)|(1<<MUX1)|(1<<MUX0))) | (((1<<MUX4)|(1<<MUX3)|(1<<MUX2)|(1<<MUX1)|(1<<MUX0)) & 0b00000111));
			  break;
			  
			  default:
			  break;
		 }
		
 }
 
 
 //When the following function is called, the ADC Conversion Complete Interrupt is activated.
static void adc_enable_interrupts(void)
 {
	  ADCSRA |= (1 << ADIE);
	  sei();
 }

static void adc_disable_interrupts(void)
{
	ADCSRA &= ~(1 << ADIE);
}

static void adc_enable_auto_triggerig(void)
 {
	  ADCSRA |= (1 << ADATE);
 }
 
 
 static void adc_disable_auto_triggerig(void)
  {
	  ADCSRA &= ~(1 << ADATE);
  }
  
 
 static void adc_select_auto_triggering_source(adc_auto_triggering_source_t adc_auto_triggering_source)
 {
	
		 switch (adc_auto_triggering_source)
		 {
			 case ADC_FREE_RUNNING_MODE:
			 adc_disable_auto_triggerig();
			 ADMUX = ((ADMUX & ~((1<<ADTS2)|(1<<ADTS1)|(1<<ADTS0)))) | ((((1<<ADTS2)|(1<<ADTS1)|(1<<ADTS0)) & 0b00000000));
			 break;
			 
			 case ADC_ANALOG_COMPARATOR:
			 adc_enable_auto_triggerig();
			 ADMUX = ((ADMUX & ~((1<<ADTS2)|(1<<ADTS1)|(1<<ADTS0)))) | ((((1<<ADTS2)|(1<<ADTS1)|(1<<ADTS0)) & 0b00000001));
			 break;
			 
			 case ADC_EXTERNAL_INTERRUPT_REQUEST_0:
			 ADMUX = ((ADMUX & ~((1<<ADTS2)|(1<<ADTS1)|(1<<ADTS0)))) | ((((1<<ADTS2)|(1<<ADTS1)|(1<<ADTS0)) & 0b00000010));
			 adc_enable_auto_triggerig();
			 break;
			 
			 case ADC_TIMER_COUNTER_0_COMPARE_MATCH:
			 ADMUX = ((ADMUX & ~((1<<ADTS2)|(1<<ADTS1)|(1<<ADTS0)))) | ((((1<<ADTS2)|(1<<ADTS1)|(1<<ADTS0)) & 0b00000011));
			 adc_enable_auto_triggerig();
			 break;
			 
			 case ADC_TIMER_COUNTER_0_OVERFLOW:
			 ADMUX = ((ADMUX & ~((1<<ADTS2)|(1<<ADTS1)|(1<<ADTS0)))) | ((((1<<ADTS2)|(1<<ADTS1)|(1<<ADTS0)) & 0b00000100));
			 adc_enable_auto_triggerig();
			 break;
			 
			 case ADC_TIMER_COUNTER_1_COMPARE_MATCH_B:
			 ADMUX = ((ADMUX & ~((1<<ADTS2)|(1<<ADTS1)|(1<<ADTS0)))) | ((((1<<ADTS2)|(1<<ADTS1)|(1<<ADTS0)) & 0b00000101));
			 adc_enable_auto_triggerig();
			 break;
			 
			 case ADC_TIMER_COUNTER_1_OVERFLOW:
			 ADMUX = ((ADMUX & ~((1<<ADTS2)|(1<<ADTS1)|(1<<ADTS0)))) | ((((1<<ADTS2)|(1<<ADTS1)|(1<<ADTS0)) & 0b00000110));
			 adc_enable_auto_triggerig();
			 break;
			 
			 case ADC_TIMER_COUNTER_1_CAPTURE_EVENT:
			 ADMUX = ((ADMUX & ~((1<<ADTS2)|(1<<ADTS1)|(1<<ADTS0)))) | ((((1<<ADTS2)|(1<<ADTS1)|(1<<ADTS0)) & 0b00000111));
			 adc_enable_auto_triggerig();
			 break;
			 
			 default:
			 break;
		 }
	 
 }
 


/*----------------------------------------------------------------
--------------------- End of File --------------------------------
----------------------------------------------------------------*/
