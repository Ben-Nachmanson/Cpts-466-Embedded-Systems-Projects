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
#include "Sound.h"
#include "Switch.h"
#include "UART.h"
#include <string.h>

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode
float converTemp(int dac);

volatile unsigned long ADCvalue;
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
int main(void){unsigned long volatile delay, input,previous,i;
  DisableInterrupts(); 
	PLL_Init(); 	// 80 MHz
	UART_Init();
	UART_OutString("Bluetooth Activated \n");	// bus clock at 80 MHz
  ADC0_InitSWTriggerSeq3_Ch1();         // ADC initialization PE2/AIN1
  SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOF; // activate port F
  delay = SYSCTL_RCGC2_R;
  GPIO_PORTF_DIR_R = 0x0E;             // make PF2 out (built-in LED)
  //GPIO_PORTF_AFSEL_R &= ~0x0E;          // disable alt funct on PF2
   GPIO_PORTF_PUR_R = 0x11;          // enable pullup resistors on PF4,PF0       
  GPIO_PORTF_DEN_R = 0x1F;          // 7) enable digital pins PF4-PF0        
                                        // configure PF2 as GPIO
  GPIO_PORTF_PCTL_R = (GPIO_PORTF_PCTL_R&0xFFFF0FF)+0x00000000;
  //GPIO_PORTF_AMSEL_R = 0;               // disable analog functionality on PF
	
	Switch_Init();       // Port F is onboard switches, LEDs, profiling
  Sound_Init(50000);   // initialize SysTick timer, 100 Hz
  // Initial testing, law of superposition
  DAC_Out(1);
  DAC_Out(2);
  DAC_Out(4);
// static testing, single step and record Vout
  for(i=0;i<8;i++)
	{
    DAC_Out(i);
  }

	  ADCvalue = ADC0_InSeq3();
		temp = convertTemp(ADCvalue);
		previous = temp;
  while(1){


    ADCvalue = ADC0_InSeq3();
		temp = convertTemp(ADCvalue);

		if(temp!=previous)
		{
			UART_OutString("Temp:\n");
			UART_OutUDec(temp);
			UART_OutString("\n");
			OutCRLF();
			previous = temp;
		
		}
		
		if(temp < 70) //green - 100hz wave
		{
			GPIO_PORTF_DATA_R = 0x08;
			EnableInterrupts();
			Sound_Init(50000);
			
			Delay10ms();
			DisableInterrupts();
		}
	
		else if( temp >= 70&&temp <75) // yellow - 200hz wave
		{
		   GPIO_PORTF_DATA_R =  0x0A;
			EnableInterrupts();
			Sound_Init(25000);
	
			
			Delay10ms();
			DisableInterrupts();
	
		}
		else// red - 400 hz		
		{	
			//GPIO_PORTF_DATA_R = 0x00;
			GPIO_PORTF_DATA_R = 0x02;
			EnableInterrupts();
			Sound_Init(12500);
			
			
			Delay10ms();
			DisableInterrupts();
			
		}
	
		//GPIO_PORTF_DATA_R = 0x02;
    //GPIO_PORTF_DATA_R &= ~0x04;
    for(delay=0; delay<100000; delay++){};
  }
}
