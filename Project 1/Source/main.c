// Multicolor LED tolggles between red-green. 
// When SW1 pressed the LED toggles blue-green.
// Demo Code of P1
// 0.Documentation Section 
// main.c
// Lab2_HelloLaunchPad, Input from PF4, output to PF3,PF2,PF1 (LED)

// LaunchPad built-in hardware
// SW1 left switch is negative logic PF4 on the Launchpad
// SW2 right switch is negative logic PF0 on the Launchpad
// red LED connected to PF1 on the Launchpad
// blue LED connected to PF2 on the Launchpad
// green LED connected to PF3 on the Launchpad

// 1. Pre-processor Directives Section
// Constant declarations to access port registers using 
// symbolic names instead of addresses
//#include "TExaS.h"
#define GPIO_PORTF_DATA_R       (*((volatile unsigned long *)0x400253FC))
#define GPIO_PORTF_DIR_R        (*((volatile unsigned long *)0x40025400))
#define GPIO_PORTF_AFSEL_R      (*((volatile unsigned long *)0x40025420))
#define GPIO_PORTF_PUR_R        (*((volatile unsigned long *)0x40025510))
#define GPIO_PORTF_DEN_R        (*((volatile unsigned long *)0x4002551C))
#define GPIO_PORTF_LOCK_R       (*((volatile unsigned long *)0x40025520))
#define GPIO_PORTF_CR_R         (*((volatile unsigned long *)0x40025524))
#define GPIO_PORTF_AMSEL_R      (*((volatile unsigned long *)0x40025528))
#define GPIO_PORTF_PCTL_R       (*((volatile unsigned long *)0x4002552C))
#define SYSCTL_RCGC2_R          (*((volatile unsigned long *)0x400FE108))

// 2. Declarations Section
//   Global Variables
unsigned long In; // input from PF0
unsigned long twoIn; // input from PF4

//   Function Prototypes
void PortF_Init(void);
void Delay(int secondsMultiplier);
void PinkDelay(void); 
//void EnableInterrupts(void);


// 3. Subroutines Section
// MAIN: Mandatory for a C Program to be executable
int main(void){    
	
  PortF_Init();        // Call initialization of port PF4 PF2    
  while(1){
		In = GPIO_PORTF_DATA_R&0x10; // read PF4 into In
		twoIn = GPIO_PORTF_DATA_R&0x01; // read PF0 into In2
		Delay(2);
		if(In == 0x00 && twoIn == 0x00)
		{
			GPIO_PORTF_DATA_R = 0x06; // LED is pink
			PinkDelay();
		}
    else if(In == 0x00) // zero means SW1 is pressed (Negative Logic, On = 0)
		{ 
      GPIO_PORTF_DATA_R = 0x02;  // LED is blue (Set the third pin of portF which is connected to the blue LED)
			Delay(10);
		}
		else if(twoIn == 0x00) //switch 2 is pressed
		{
			GPIO_PORTF_DATA_R = 0x04; // LED is blue
			Delay(20);
		}
		else
		{
			GPIO_PORTF_DATA_R = 0; // LED dark                   
    }
	}
}

// Subroutine to initialize port F pins for input and output
// PF4 and PF0 are input SW1 and SW2 respectively
// PF3,PF2,PF1 are outputs to the LED
// Inputs: None
// Outputs: None
// Notes: These five pins are connected to hardware on the LaunchPad
void PortF_Init(void)
	{ 
	volatile unsigned long delay;
  SYSCTL_RCGC2_R |= 0x00000020;     // 1) F clock
  delay = SYSCTL_RCGC2_R;           // delay   
  GPIO_PORTF_LOCK_R = 0x4C4F434B;   // 2) unlock PortF PF0  
  GPIO_PORTF_CR_R = 0x1F;           // allow changes to PF4-0       
  //GPIO_PORTF_AMSEL_R = 0x00;        // 3) disable analog function
  GPIO_PORTF_PCTL_R = 0x00000000;   // 4) GPIO clear bit PCTL  
  GPIO_PORTF_DIR_R = 0x0E;          // 5) PF4,PF0 input, PF3,PF2,PF1 output   
  //GPIO_PORTF_AFSEL_R = 0x00;        // 6) no alternate function
  GPIO_PORTF_PUR_R = 0x11;          // enable pullup resistors on PF4,PF0       
  GPIO_PORTF_DEN_R = 0x1F;          // 7) enable digital pins PF4-PF0        
}
// Color    LED(s) PortF
// dark     ---    0
// red      R--    0x02
// blue     --B    0x04
// green    -G-    0x08
// yellow   RG-    0x0A
// sky blue -GB    0x0C
// white    RGB    0x0E
// pink     R-B    0x06

// Subroutine to wait 0.1 sec. 
// Inputs: None
// Outputs: None
// Notes: It is an estimation based on the clock frequency and the number of cycles per instruction.
void Delay(int secondsMultiplier){
	unsigned long volatile time;
  time = 145448 * secondsMultiplier;  // 0.1sec
  while(time){
		time--;
  }
}
// This will loop the pink and change the LED to green and go for one second and go dark after.
void PinkDelay(void){
	
	unsigned long volatile time;
	
	time = 145448 * 40; // 0.1 ms * 40 = 4 s
	
	while(time)
	{
		In = GPIO_PORTF_DATA_R&0x10; // access to SW1
		
		if(In == 0x00)
		{
			GPIO_PORTF_DATA_R = 0x08; // LED is green
			time = 145448 *10; // 0.1 ms * 10 = 1 s
		}
		
		time--;
	}
	
}

