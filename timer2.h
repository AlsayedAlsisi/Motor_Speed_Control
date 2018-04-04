/*
 * timer2.h
 *
 * Created: 04-Mar-2018
 * Author: Alsayed Alsisi
 * Contact Details:
 - (+2)01066769510
 - alsayed.alsisi@gmail.com
 */


#ifndef TIMER2_H_
#define TIMER2_H_

/*----------------------------------------------------------------
--------------------- File Inclusions ----------------------------
----------------------------------------------------------------*/

#include <stdint.h>



/*----------------------------------------------------------------
--------------------- Public Function Prototypes ----------------
----------------------------------------------------------------*/
//To configure the timer to work in PWM mode.
extern void timer2_pwm_init(void);

// To start the PWM signal:
extern void timer2_pwm_start(void);

//To change the duty cycle as required. And note that 0% duty cycle means no output on the pwm OC2 pin.
extern void timer2_pwm_duty_cycle_set(uint8_t duty_cycle_percentage);





#endif /* TIMER2_H_ */