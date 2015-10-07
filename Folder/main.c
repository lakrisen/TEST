#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h> 

#include "uart.h"
#include "line_buffer.h"
#include "circ_buffer.h"
#include "encoder.h"
#include "controller.h"
//#include "motor.h"
#include "sysid.h"
#include "motor_test.h"
static LB_T lb;


// TODO: Replace these with receive buffer declaration
static volatile char ch;
static volatile bool ch_waiting;




int main(void)
{
	float x = 0.0;
	float y = 0.0;
	float xy = 0.0;
	float theta = 0;	//actual pendulumn angle 
	float vel =  0;		//actual cart veloctiy 
	float velref = 0;	//desired cart velocity 
	float ctrl = 0; 	//output control signal 
	float ctrl_temp = 0;
	int log_count = 0;
	float time = 0; 
	int motor_dir = 0;

	
	//Initialise the things 
	lb_init(&lb);
	DDRD |= (1<<6);
	uart_init(); 	// init USART
	sei();  		// enable interrupts
	enc_init();
	ctrl_init();
	PWM_init();
	//motor_test();
	printf_P(PSTR("Hello World\n"));

	//motor_init();
	sysid_init();

	// Wait a second at startup
	_delay_ms(1000);

	// send initial string
	printf_P(PSTR("Mr. Bond, what would you like to do today?!\n"));


	//Infinite loop
	for (;/*ever*/;)
	{

		/*
		if (event_count > 0) // Operation pending, do it right now (high priority)
			{
					event_count = 0; // Reset counter
					
					
					
					if(log_count>0){
					printf_P(PSTR("%f ,%f\n"),time, adc_read());
					// TODO: Handle sampling and sending of one line of data
					// Note: If we have sent the number requested samples,
					// disable the data logging
					time +=0.01;
					log_count--;


					if(log_count ==0){TIMSK &=~(1<<OCIE0);
					time = 0;}

					}
			if (event_count > 0)
				{
					
					// If we ever get here, the sampling/sending step above is taking
					// too long to fit in one sampling interval!
					printf_P(PSTR("ERROR: Sampling interval too long\n"));
					// TODO: Print an error message and disable the data logging
				}
			}
			
			else // No pending operation, do low priority tasks
			{
			// TODO: Handle serial command processing here
			


		*/

		// Do some pretend processing
		//_delay_ms(250);

			while (uart_avail())
				{
					char c = uart_getc();
					// TODO: Transfer character to line buffer and handle overflow.
					//lb_append(&lb, c);
					// TODO: Process command if line buffer is ready ...
					if (lb_append(&lb, c) == LB_BUFFER_FULL)
						{
							lb_init(&lb); // Clear line
							printf_P(PSTR("\nMax line length exceeded\n"));
						}

				
				}



	
		// Process command if line buffer is terminated by a line feed or carriage return
		if (lb_line_ready(&lb))
		{
			// Note: The following is a terrible way to process strings from the user
			//       See recommendations section of the lab guide for a better way to
			//       handle commands with arguments, which scales well to a large
			//       number of commands.
			if (!strncmp_P(lb_gets(&lb), PSTR("help"), 4)) //use lbgets to read data from the buffer and compare 
			{
				printf_P(PSTR(
					"MCHA3000 Serial help.\n"
					"Instruction Table.\n"
					"x = (set value of x)\n"
					"x? (print value of x)\n"
					"count? (query encoder count)\n"
					"reset (reset encoder count)\n"
					
					));
			}


		/*	else if (!strncmp_P(lb_gets(&lb), PSTR("logv0 "), 6))		//Set the X value
			{
				log_count = atof(lb_gets_at(&lb, 6));
				TIMSK |= 1<<OCIE0; //enable interrupt on compare
				printf_P(PSTR("Time (sec), Voltage (V)\n"));

				//printf_P(PSTR("x set to %f\n"), x);
			}*/
	
			else if (!strncmp_P(lb_gets(&lb), PSTR("x="), 2))		//Set the X value
			{
				x = atof(lb_gets_at(&lb, 2));
				//printf_P(PSTR("x set to %f\n"), x);
			}

			else if (!strncmp_P(lb_gets(&lb), PSTR("x?"), 2))		//query the X value
			{
				printf_P(PSTR("x is %f\n"), x);
			}

			else if (!strncmp_P(lb_gets(&lb), PSTR("y="), 2))		//set the value of Y value
			{
				y = atof(lb_gets_at(&lb, 2));
				//printf_P(PSTR("y set to %f\n"), y);
			}

			else if (!strncmp_P(lb_gets(&lb), PSTR("xy?"), 3))		//set the value of XxY value
			{
				xy = x*y;
				printf_P(PSTR("xy is %f\n"), xy);
			}
			
			else if (!strncmp_P(lb_gets(&lb), PSTR("theta="), 6))		//Set the actual theta value
			{
				theta = atof(lb_gets_at(&lb, 6));						

			}

			else if (!strncmp_P(lb_gets(&lb), PSTR("vel="), 4))		//Set the actual cart velocity value
			{
				vel = atof(lb_gets_at(&lb, 4));						

			}
			else if (!strncmp_P(lb_gets(&lb), PSTR("velref="), 7))		//Set the  requested value for velocity
			{
				velref = atof(lb_gets_at(&lb, 7));						

			}
			else if (!strncmp_P(lb_gets(&lb), PSTR("ctrl?"), 5))		//compute the control 
			{
				
				ctrl_temp = velocity_controller(velref-vel);
				//printf_P(PSTR("%g\n"), ctrl_temp);
				ctrl = angle_controller(ctrl_temp-theta);
				printf_P(PSTR("%g\n"), ctrl);
			}




			else if (!strncmp_P(lb_gets(&lb), PSTR("count?"), 6))	//query the count 	
			{
				printf_P(PSTR("count1 =  %d\n"), enc1_read());
				printf_P(PSTR("count2 =  %d\n"), enc2_read());
			}

			else if (!strncmp_P(lb_gets(&lb), PSTR("reset"), 5))	//reset the encoder 
			{
				enc_reset();
				printf_P(PSTR("count1 =  %d\n"), enc1_read());
				printf_P(PSTR("count2 =  %d\n"), enc2_read());
			}
			
			else if (!strncmp_P(lb_gets(&lb), PSTR("motor "), 5))		//display atmega pin configuration 
			{
				//printf_P(PSTR("current =  %f\n"), motor_current());
				//printf_P(PSTR("adc =  %d\n"), test());
				printf_P(PSTR("Motor Command Accepted\n"));
				motor_dir = atof(lb_gets_at(&lb, 5));
				motor_test(motor_dir);

			}
			else if (!strncmp_P(lb_gets(&lb), PSTR("pin"), 5))		//display atmega pin configuration 
			{
				printf_P(PSTR(
					"Pin Configuration.\n"

				"   Status LED: (XCK/T0)PB0 <-> 1 +---\\_/---+40 <-- PA0(ADC0) :Angle potentiometer\n"
				"                   (T1)PB1 <-> 2 |         |39 <-- PA1(ADC1) :Z accelerometer\n"
				"            (INT2/AIN0)PB2 <-> 3 | A       |38 <-- PA2(ADC2) :X accelerometer\n"
				"             (OC0/AIN1)PB3 <-> 4 | T       |37 <-- PA3(ADC3) :Y gyro\n"
				"                  (!SS)PB4 <-> 5 | M       |36 <-- PA4(ADC4) :Y gyro (4.5x sens.)\n"
				"                 (MOSI)PB5 <-> 6 | E       |35 <-> PA5(ADC5)\n"
				"                 (MISO)PB6 <-> 7 | L       |34 <-> PA6(ADC6)\n"
				"                  (SCK)PB7 <-> 8 |         |33 <-- PA7(ADC7) :Gyro reference\n"
				"                    !RESET --> 9 | A       |32 <-- AREF :3.3V\n"
				"                       VCC --- 10| T       |31 --- GND\n"
				"                       GND --- 11| m       |30 --- AVCC\n"
				"                     XTAL2 <-- 12| e       |29 <-> PC7(TOSC2)\n"
				"                     XTAL1 --> 13| g       |28 <-> PC6(TOSC1)\n"
				"                  (RXD)PD0 --> 14| a       |27 <-> PC5(TDI)\n"
				"                  (TXD)PD1 <-- 15| 3       |26 <-> PC4(TDO)\n"
				"         E-Stop: (INT0)PD2 --> 16| 2       |25 <-> PC3(TMS)\n"
				"   Limit switch: (INT1)PD3 --> 17|         |24 <-> PC2(TCK)\n"
				"                 (OC1B)PD4 <-> 18|         |23 <-> PC1(SDA)\n"
				"                 (OC1A)PD5 <-> 19|         |22 --> PC0(SCL) :SyRen 10 S2 (dir.)\n"
				"        Az IMU5: (ICP1)PD6 <-- 20+---------+21 --> PD7(OC2) :SyRen 10 S1 (speed)\n"
					
					));
			}

			else
			{
				printf_P(PSTR("Unknown command: \"%s\"\n"), lb_gets(&lb));
			}
			
			
			
			
			// Reset line buffer
			lb_init(&lb);
		
		}
		}
	
	
	
	
	return 0;
}

