#include "PLL.h"
#include "SysTick.h"

#define LIGHT                   (*((volatile unsigned long *)0x400050FC)) //bits 5-0
#define GPIO_PORTB_OUT          (*((volatile unsigned long *)0x400050FC)) // bits 5-0
#define GPIO_PORTB_DIR_R        (*((volatile unsigned long *)0x40005400))
#define GPIO_PORTB_AFSEL_R      (*((volatile unsigned long *)0x40005420))
#define GPIO_PORTB_DEN_R        (*((volatile unsigned long *)0x4000551C))
#define GPIO_PORTB_AMSEL_R      (*((volatile unsigned long *)0x40005528))
#define GPIO_PORTB_PCTL_R       (*((volatile unsigned long *)0x4000552C))
#define GPIO_PORTE_IN           (*((volatile unsigned long *)0x4002400C)) // bits 1-0
	
//#define MainS              (*((volatile unsigned long *)0x40024004))
//#define SpringS                 (*((volatile unsigned long *)0x40024008))
//#define PedS                 (*((volatile unsigned long *)0x40024010))
	
#define SENSOR (*((volatile unsigned long *)0x4002401C))

#define GPIO_PORTE_DIR_R        (*((volatile unsigned long *)0x40024400))
#define GPIO_PORTE_AFSEL_R      (*((volatile unsigned long *)0x40024420))
#define GPIO_PORTE_DEN_R        (*((volatile unsigned long *)0x4002451C))
#define GPIO_PORTE_AMSEL_R      (*((volatile unsigned long *)0x40024528))
#define GPIO_PORTE_PCTL_R       (*((volatile unsigned long *)0x4002452C))
#define SYSCTL_RCGC2_R          (*((volatile unsigned long *)0x400FE108))
#define SYSCTL_RCGC2_GPIOE      0x00000010  // port E Clock Gating Control
#define SYSCTL_RCGC2_GPIOB      0x00000002  // port B Clock Gating Control

//Initialize PLL and configure the clock frequency using SYSDIV2
void PLL_Init(void);
// Initialize SysTick with busy wait running at bus clock.
void SysTick_Init(void);
// Time delay using busy wait. This assumes 80 MHz system clock.
void SysTick_Wait10ms(unsigned long delay);

//MAIN ST

void goN_f(void)
{	
	LIGHT = 0x21; // 
	SysTick_Wait10ms(50);

}
void slowN_f(void)
{

	LIGHT = 0x22;
	SysTick_Wait10ms(50);
}

//Spring
void goE_f(void)
{
	LIGHT = 0x0C;
	SysTick_Wait10ms(50);
	
}
void slowE_f(void)
{
	LIGHT = 0x14;
	SysTick_Wait10ms(50);
}
//PED
void goP_f(void){
LIGHT = 0x25;	
SysTick_Wait10ms(500);
}

void hurryP_f(void){
unsigned short i = 0;	
	LIGHT = 0x24;	
while(i<10){
	SysTick_Wait10ms(50);
	LIGHT ^= 0x02;
	i++;
}
}

/* ped -> in3 = PE2
spring -> in2 = PE1
main -> in = PE0

MAIN ST LIGHTS
PB0 = GREEN
PB1 = YELLOW
PB2 = RED

SPRING ST LIGHTS
PB3 = GREEN
PB4 = YELLOW
PB5 = RED **CHECK THE HARDWARE

PEDESTRIAN LIGHT
PB5.....

*/
// Linked data structure
struct State {
	void (*CmdPt)(void);   // output function 
  unsigned long Next[8];}; 
typedef const struct State STyp;
#define goN   0
#define slowN 1
#define goE   2
#define slowE 3
#define goP   4
#define hurryP 5


STyp FSM[6] = 
	{
		{&goN_f,{goN,goN,slowN,slowN,slowN,slowN,slowN,slowN}},
		{&slowN_f,{goE,goE,goE,goE,goP,goP,goP,goP}},
		{&goE_f,{goE,slowE,goE,goN,slowE,slowE,slowE,slowE}},
		{&slowE_f,{goN,goN,goN,goN,goP,goP,goP,goP}},
		{&goP_f,{hurryP,hurryP,hurryP,hurryP,hurryP,hurryP,hurryP,hurryP}},
		{&hurryP_f,{goN,goN,goE,goE,goN,goN,goE,goN}}};
			
unsigned long S;  // index to the current state 
unsigned long Input; 
int main(void){ volatile unsigned long delay;
  PLL_Init();       // 80 MHz, Program 10.1
  SysTick_Init();   // Program 10.2
  SYSCTL_RCGC2_R |= 0x12;      // 1) B E
  delay = SYSCTL_RCGC2_R;      // 2) no need to unlock
  GPIO_PORTE_AMSEL_R &= ~0x07; // 3) disable analog function on PE2-0
  GPIO_PORTE_PCTL_R &= ~0x000000FF; // 4) enable regular GPIO
  GPIO_PORTE_DIR_R &= ~0x07;   // 5) inputs on PE2-0
  GPIO_PORTE_AFSEL_R &= ~0x07; // 6) regular function on PE2-0
  GPIO_PORTE_DEN_R |= 0x07;    // 7) enable digital on PE2-0
  GPIO_PORTB_AMSEL_R &= ~0x3F; // 3) disable analog function on PB5-0
  GPIO_PORTB_PCTL_R &= ~0x00FFFFFF; // 4) enable regular GPIO
  GPIO_PORTB_DIR_R |= 0x3F;    // 5) outputs on PB5-0
  GPIO_PORTB_AFSEL_R &= ~0x3F; // 6) regular function on PB5-0
  GPIO_PORTB_DEN_R |= 0x3F;    // 7) enable digital on PB5-0
  S = goN;
  while(1){
		
		(FSM[S].CmdPt)(); // output + delay
		
    Input = SENSOR;     // read sensors
		
    S = FSM[S].Next[Input]; // next states 
  }
}

