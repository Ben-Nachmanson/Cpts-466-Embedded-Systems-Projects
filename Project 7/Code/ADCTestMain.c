// ADCTestMain.c
// Runs on LM4F120/TM4C123
// This program periodically samples ADC channel 1 and stores the
// result to a global variable that can be accessed with the JTAG
// debugger and viewed with the variable watch feature.
// Daniel Valvano
// October 20, 2013

/* This example accompanies the book
   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2013

 Copyright 2013 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */

// input signal connected to PE2/AIN1

#include "ADCSWTrigger.h"
#include "tm4c123gh6pm.h"
#include "PLL.h"
#include "Switch.h"
#include "UART.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode
float convertTemp(int dac);

volatile float temp;
// The digital number ADCvalue is a representation of the voltage on PE4 
// voltage  ADCvalue
// 0.00V     0
// 0.75V    1024
// 1.50V    2048
// 2.25V    3072
// 3.00V    4095
float convertTemp(int dac)
{
	float temp = 0;
	
	temp = (dac*3.30)/4095;
	
	temp = ( temp - .5)*100;
	
	return temp;
}
//---------------------OutCRLF---------------------
// Output a CR,LF to UART to go to a new line
// Input: none
// Output: none
void OutCRLF(void){
  UART_OutChar(CR);
  UART_OutChar(LF);
}
//void Switch_Init(void){  unsigned long volatile delay;
//  SYSCTL_RCGC2_R |= 0x00000020; // (a) activate clock for port F **
//  delay = SYSCTL_RCGC2_R;
//  GPIO_PORTF_LOCK_R = 0x4C4F434B; // unlock GPIO Port F
//  GPIO_PORTF_CR_R = 0x1F;         // allow changes to PF4-0
//  GPIO_PORTF_DIR_R = 0x0E;    // (c) PF4,PF0 input, PF3,PF2,PF1 output
//  GPIO_PORTF_AFSEL_R &= ~0x11;  //     disable alt funct on PF4,0
//  GPIO_PORTF_DEN_R = 0x1F;     //     enable digital I/O on PF4-0
//  GPIO_PORTF_PCTL_R &= ~0x000F000F; //  configure PF4,0 as GPIO
//  GPIO_PORTF_AMSEL_R &= ~0x11;  //     disable analog functionality on PF4,0
//  GPIO_PORTF_PUR_R |= 0x11;     //     enable weak pull-up on PF4,0
//  GPIO_PORTF_IS_R &= ~0x11;     // (d) PF4,PF0 is edge-sensitive
//  GPIO_PORTF_IBE_R &= ~0x11;    //     PF4,PF0 is not both edges
//  GPIO_PORTF_IEV_R &= ~0x11;    //     PF4,PF0 falling edge event
//  GPIO_PORTF_ICR_R = 0x11;      // (e) clear flags 4,0
//  GPIO_PORTF_IM_R |= 0x11;      // (f) arm interrupt on PF4,PF0
//  NVIC_PRI7_R = (NVIC_PRI7_R&0xFF00FFFF)|0x00400000; // (g) priority 2
//  NVIC_EN0_R = 0x40000000;      // (h) enable interrupt 30 in NVIC
//	 
//}

int main(void){unsigned long volatile delay, input,previous,i, in;

	unsigned long thermometer,photoCell, fill;
	DisableInterrupts(); 
	PLL_Init(); 	// 80 MHz
	Switch_Init();
	UART_Init();
	//UART_OutString("Temp, photo, Light\n");	// bus clock at 80 MHz
  ADC_Init298();        // initialize ADC to sample AIN2 (PE1), AIN9 (PE4), AIN8 (PE5)
  

  while(1){
	//temp = ain2, photo = ain9
	ADC_In298(&thermometer, &photoCell,&fill); // sample AIN2(PE1), AIN9 (PE4)
	temp = convertTemp(thermometer);
	
//	UART_OutUDec(temp);
//	UART_OutString(",");
//	UART_OutUDec(photoCell);
//	UART_OutString(",");
//	in = GPIO_PORTF_DATA_R&0x01;
//	if(in != 0)
//	{
//		UART_OutString("Off\n");
//		
//	}
//	else
//	{
//		UART_OutString("On\n");
//	}

//		
	
	if(photoCell < 97.5)
	{
		if(photoCell < 27.5)
		{
			UART_OutString("Off\n");
		}
		else if(photoCell >= 27.5)
		{
			if(photoCell > 81.5)
			{
				UART_OutString("On\n");
			}
			else
			{
				UART_OutString("Off\n");
			}
			
		}
		
	}
	else if(photoCell >=97.5)
	{
		UART_OutString("On\n");
	}

	
			for(delay=0; delay<100000; delay++){};

}
	}
