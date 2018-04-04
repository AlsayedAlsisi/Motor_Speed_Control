/*
 * Volume Control.c
 *
 * Created: 04-Apr-18 3:41:51 PM
 * Author : Alsayed
 */ 

#include <avr/io.h>
#include "timer2.h"
#include "adc.h"
#include "volumeControl.h"
int main(void)
{
    adc_init();
	timer2_pwm_init();
	timer2_pwm_start();
    while (1) 
    {
		volume_control_update();
    }
}

