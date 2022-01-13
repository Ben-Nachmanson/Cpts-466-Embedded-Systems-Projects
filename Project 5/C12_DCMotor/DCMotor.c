// DCMotor.c
// Runs on LM4F120 or TM4C123
// Use SysTick interrupts to implement a software PWM to drive
// a DC motor at a given duty cycle.  The built-in button SW1
// increases the speed, and SW2 decreases the speed.
// Daniel Valvano, Jonathan Valvano
// August 6, 2013

/* This example accompanies the book
   "Embedded Systems: Introduction to ARM Cortex M Microcontrollers",
   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2013
   "Embedded Systems: Real Time Interfacing to ARM Cortex M Microcontrollers",
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

// PA5 connected to DC motor interface
#include "UART.h"
#include "PLL.h"
#include "tm4c123gh6pm.h"
#include <string.h>

	
// basic functions defined at end of startup.s
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode

unsigned long H,L;
unsigned long onBoardButton;

void Motor_Init(void){
  SYSCTL_RCGC2_R |= 0x00000001; // activate clock for port A
  H = L = 40000;                // 50%
  GPIO_PORTA_AMSEL_R &= ~0x60;      // disable analog functionality on PA5
	GPIO_PORTA_PCTL_R &= ~0x0FF00000; // configure PA5,PA6 as GPIO
  GPIO_PORTA_DIR_R |= 0x20;     // make PA5 out
  GPIO_PORTA_DR8R_R |= 0x20;    // enable 8 mA drive on PA5
  GPIO_PORTA_AFSEL_R &= ~0x60;  // disable alt funct on PA5,PA6
  GPIO_PORTA_DEN_R |= 0x60;     // enable digital I/O on PA5,PA6
  GPIO_PORTA_DATA_R &= ~0x60;   // make PA5 low
  NVIC_ST_CTRL_R = 0;           // disable SysTick during setup
  NVIC_ST_RELOAD_R = L-1;       // reload value for 500us
  NVIC_ST_CURRENT_R = 0;        // any write to current clears it
  NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R&0x00FFFFFF)|0x40000000; // priority 2
  NVIC_ST_CTRL_R = 0x00000007;  // enable with core clock and interrupts
	
	GPIO_PORTA_CR_R = 0x3F;         // allow changes to PA6-0
}
//---------------------OutCRLF---------------------
// Output a CR,LF to UART to go to a new line
// Input: none
// Output: none
void OutCRLF(void){
  UART_OutChar(CR);
  UART_OutChar(LF);
}
unsigned long uartValue = 0;
void SysTick_Handler(void){
	
	
  if(GPIO_PORTA_DATA_R&0x20){   // toggle PA5
    GPIO_PORTA_DATA_R &= ~0x20; // make PA5 low
    NVIC_ST_RELOAD_R = L-1;     // reload value for low phase
  } else{
    GPIO_PORTA_DATA_R |= 0x20;  // make PA5 high
    NVIC_ST_RELOAD_R = H-1;     // reload value for high phase
  }
}
void Switch_Init(void){  unsigned long volatile delay;
  SYSCTL_RCGC2_R |= 0x00000020; // (a) activate clock for port F **
  delay = SYSCTL_RCGC2_R;
  GPIO_PORTF_LOCK_R = 0x4C4F434B; // unlock GPIO Port F
  GPIO_PORTF_CR_R = 0x1F;         // allow changes to PF4-0
  GPIO_PORTF_DIR_R = 0x0E;    // (c) PF4,PF0 input, PF3,PF2,PF1 output
  GPIO_PORTF_AFSEL_R &= ~0x11;  //     disable alt funct on PF4,0
  GPIO_PORTF_DEN_R = 0x1F;     //     enable digital I/O on PF4-0
  GPIO_PORTF_PCTL_R &= ~0x000F000F; //  configure PF4,0 as GPIO
  GPIO_PORTF_AMSEL_R &= ~0x11;  //     disable analog functionality on PF4,0
  GPIO_PORTF_PUR_R |= 0x11;     //     enable weak pull-up on PF4,0
  GPIO_PORTF_IS_R &= ~0x11;     // (d) PF4,PF0 is edge-sensitive
  GPIO_PORTF_IBE_R &= ~0x11;    //     PF4,PF0 is not both edges
  GPIO_PORTF_IEV_R &= ~0x11;    //     PF4,PF0 falling edge event
  GPIO_PORTF_ICR_R = 0x11;      // (e) clear flags 4,0
  GPIO_PORTF_IM_R |= 0x11;      // (f) arm interrupt on PF4,PF0
  NVIC_PRI7_R = (NVIC_PRI7_R&0xFF00FFFF)|0x00400000; // (g) priority 2
  NVIC_EN0_R = 0x40000000;      // (h) enable interrupt 30 in NVIC
	 
}

// L range: 8000,16000,24000,32000,40000,48000,56000,64000,72000
// power:   10%    20%   30%   40%   50%   60%   70%   80%   90%
void GPIOPortF_Handler(void){ // called on touch of either SW1 or SW2
   		
	if(GPIO_PORTF_RIS_R&0x10 && GPIO_PORTF_RIS_R&0x01)
	{
		L = 8000;
		GPIO_PORTF_DATA_R = 0x00;
		GPIO_PORTF_ICR_R = 0x10;
		 GPIO_PORTF_ICR_R = 0x01;  // acknowledge flag0
	}
		
	if(GPIO_PORTF_RIS_R&0x01){  // SW2 touch
    GPIO_PORTF_ICR_R = 0x01;  // acknowledge flag0
    if(L>8000) L = L-8000;    // slow down
		if(L <= 8000) GPIO_PORTF_DATA_R = 0x00; 
  }
  if(GPIO_PORTF_RIS_R&0x10){  // SW1 touch
    GPIO_PORTF_ICR_R = 0x10;  // acknowledge flag4
    if(L<72000) L = L+8000;   // speed up
		GPIO_PORTF_DATA_R = 0x02;

  }
  H = 80000-L; // constant period of 1ms, variable duty cycle

}
int main(void){
	unsigned long volatile delay;
  DisableInterrupts();  // disable interrupts while initializing
	
  PLL_Init();
  UART_Init();
	UART_OutString("Bluetooth Activated \n");	// bus clock at 80 MHz
	UART_OutString("There are speeds 1-8 \n");	// bus clock at 80 MHzx
  Motor_Init();         // output from PA5, SysTick interrupts
  Switch_Init();        // arm PF4, PF0 for falling edge interrupts
  EnableInterrupts();   // enable after all initialization are done
	GPIO_PORTF_DATA_R = 0x02;
  while(1){

    // main program is free to perform other tasks
    WaitForInterrupt(); // low power mode
		
	  // BLUETOOTH
	  UART_OutString("Enter Value(1-8) for speed \n");
	  uartValue=UART_InUDec();
    UART_OutString(" \nspeed: ");
		
		
		if(uartValue == 1) L = 8000;
		else if(uartValue == 2) L = 16000;
		else if(uartValue == 3) L = 24000;
		else if(uartValue == 4) L = 32000;
		else if(uartValue == 5) L = 40000;
		else if(uartValue == 6) L = 48000;
		else if(uartValue == 7) L = 56000;
		else if(uartValue == 8) L = 64000;
		if(L >8000) GPIO_PORTF_DATA_R = 0x02;
		else GPIO_PORTF_DATA_R = 0x00;

		UART_OutUDec(L);
		
		UART_OutString("\n");
		OutCRLF();
		uartValue = 0;
		
		
  }
}
