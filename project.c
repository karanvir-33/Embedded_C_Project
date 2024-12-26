#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include "C:\Program Files (x86)\LabJack\Drivers\LabJackUD.h"

int main()
{
	// Variable Declaration
	LJ_ERROR lj_cue;
	LJ_HANDLE lj_handle = 0;

	double thresTemp, time, tempAIN0;
	char choice;
	int buzzerState = 0;
	int buzzerActivations = 0;
	int counter = 0;

	// Open communication with LabJack and reset the pins
	lj_cue = OpenLabJack(LJ_dtU3, LJ_ctUSB, "1", 1, &lj_handle);
	lj_cue = ePut(lj_handle, LJ_ioPIN_CONFIGURATION_RESET, 0, 0, 0);

	lj_cue = AddRequest(lj_handle, LJ_ioPUT_DAC, 1, 5, 0, 0); // Led is off
	lj_cue = Go();
	
	// Program Explanation to User 
    printf("Welcome to the Temperature Monitoring Program!\n\n");
    printf("This program will monitor the temperature using a sensor connected to the LabJack device.\n");
    printf("It will alert you with a buzzer and a blue LED when the temperature exceeds a threshold that you set.\n");
    printf("The LED will turn on when the temperature is below the threshold and turn off when the temperature exceeds the threshold.\n");
    printf("The buzzer will turn on when the temperature exceeds the threshold, and turn off when it goes back below.\n");
    printf("You can choose to run the program for a specified time or until the buzzer has been activated a certain number of times.\n");
    printf("After the exit condition is met, the program will stop and both the buzzer and LED will be turned off.\n\n");
	
	printf("Please enter the threshold temperature in degrees Celsius for the buzzer to sound: ");
	scanf(" %lf", &thresTemp);
	
	printf("\nPlease enter the choice how you would you to exit the program\n");
	printf("A. Time: How many seconds you would like to run the program\n");
	printf("B. How many buzzer activations: The number of time you would like to run the buzzer until the threshold temperature has been reached\n");
	scanf(" %c", &choice);
	
	// PWM set up only once 
	lj_cue = AddRequest(lj_handle, LJ_ioPUT_CONFIG, LJ_chTIMER_COUNTER_PIN_OFFSET, 4, 0, 0); // Set Up FIO4 as PWM output
	lj_cue = AddRequest(lj_handle, LJ_ioPUT_CONFIG, LJ_chTIMER_CLOCK_BASE, LJ_tc4MHZ_DIV, 0, 0); // Set up the TCB as 4 MHz with DIV
	lj_cue = AddRequest(lj_handle, LJ_ioPUT_CONFIG, LJ_chTIMER_CLOCK_DIVISOR, 4, 0, 0); // Since we use 4MHz DIV, need the DIV	 
	lj_cue = AddRequest(lj_handle, LJ_ioPUT_CONFIG, LJ_chNUMBER_TIMERS_ENABLED, 1, 0, 0); //Enable the timer
	lj_cue = AddRequest(lj_handle, LJ_ioPUT_TIMER_MODE, 0, LJ_tmPWM8, 0, 0); // Set the TM to 8-bit	
	lj_cue = AddRequest(lj_handle, LJ_ioPUT_TIMER_VALUE, 0, 0, 0, 0); // Buzzer is off
	lj_cue = Go(); 
 
	
	switch (choice)
	{
		 case 'A': 
				printf("How many seconds you would like to run the program?\n");
				scanf(" %lf", &time);

				while ( counter < (int)((time * 1000)/200))
				{	
					lj_cue = AddRequest(lj_handle, LJ_ioGET_AIN, 0, 0, 0, 0);
					lj_cue = Go();
					lj_cue = GetResult(lj_handle, LJ_ioGET_AIN, 0, &tempAIN0);
			
					printf("The current temperature is %.2lf degrees Celsius.\n", (tempAIN0 * 100));
					
					// Temperature below threshold
					if((tempAIN0 * 100 ) < thresTemp)
					{
						lj_cue = AddRequest(lj_handle, LJ_ioPUT_TIMER_VALUE, 0, 0, 0, 0); // Buzzer Off
						lj_cue = AddRequest(lj_handle, LJ_ioPUT_DAC, 1, 0, 0, 0); // Led is On
						lj_cue = Go();
					}
					// Temperature exceeds threshold
					else if ((tempAIN0 * 100 ) > thresTemp)
					{
						lj_cue = AddRequest(lj_handle, LJ_ioPUT_TIMER_VALUE, 0, 32768, 0, 0); // Buzzer On with 50% duty cycle
						lj_cue = AddRequest(lj_handle, LJ_ioPUT_DAC, 1, 5, 0, 0); // Led is Off
						lj_cue = Go();	
						printf("Temperature is high: %.2f degrees Celsius.\n", (tempAIN0 * 100));
					}

					// Sleep for 200 ms (5 updates per second)
					Sleep(200);
					counter++;
				}
				break;
		 case 'B':
					// Buzzer activation-based exit strategy
            		printf("How many times would you like the buzzer to activate?\n");
            		scanf("%d", &buzzerActivations);
		
					while (buzzerActivations > 0) 
					{
                		lj_cue = AddRequest(lj_handle, LJ_ioGET_AIN, 0, 0, 0, 0);  // Get temperature reading from AIN0
               			lj_cue = Go();
                		lj_cue = GetResult(lj_handle, LJ_ioGET_AIN, 0, &tempAIN0);

                		// Display the current temperature
                		printf("The current temperature is %.2f degrees Celsius.\n", (tempAIN0 * 100));

                		// Temperature below threshold
                		if ((tempAIN0 * 100 ) < thresTemp) 
						{
                    		if (buzzerState == 1) 
							{
                				// Turn buzzer off (if it's currently on due to sticky behavior)
                				lj_cue = AddRequest(lj_handle, LJ_ioPUT_TIMER_VALUE, 0, 0, 0, 0);  // Buzzer off
                				lj_cue = AddRequest(lj_handle, LJ_ioPUT_DAC, 1, 0, 0, 0);  // LED on
                				lj_cue = Go();
                				buzzerState = 0;  // Reset buzzer state to off
                				printf("Buzzer turned off, temperature below threshold.\n");
            				}
                		} 
                		// Temperature exceeds threshold
                		else if ((tempAIN0 * 100 ) > thresTemp)
				 		{
                    		if (buzzerState == 0) 
							{
                				// Turn buzzer on (sticky behavior)
                				lj_cue = AddRequest(lj_handle, LJ_ioPUT_TIMER_VALUE, 0, 32768, 0, 0);  // Buzzer on (50% duty cycle)
                				lj_cue = AddRequest(lj_handle, LJ_ioPUT_DAC, 1, 5, 0, 0);  // LED off
                				lj_cue = Go();

                				buzzerState = 1;  // Set buzzer state to on (sticky)

                				printf("Temperature exceeded threshold. Buzzer activated!\n");

								buzzerActivations--;  // Decrease the remaining buzzer activations
							}

                		}

                		// Sleep for 200 ms (5 updates per second)
                		Sleep(200);
            		}

				break;
		 default:
				break;
	}

	// Turn off both LEDs and buzzer at the end
	lj_cue = AddRequest(lj_handle, LJ_ioPUT_TIMER_VALUE, 0, 0, 0, 0); // Buzzer off
	lj_cue = AddRequest(lj_handle, LJ_ioPUT_DAC, 1, 5, 0, 0); // Blue LED off 
	lj_cue = Go();	
	system("pause");
	lj_cue = ePut(lj_handle, LJ_ioPIN_CONFIGURATION_RESET, 0, 0, 0); // Turn off PWM signal 
	lj_cue = AddRequest(lj_handle, LJ_ioPUT_DIGITAL_BIT, 4, 0, 0, 0); // Set FIO4 to a DO and send a low value
	lj_cue = Go();

	Close(); // Close communication with LabJack

	return 0;

}