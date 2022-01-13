// There is a switch on PE0 which controlls an LED which is on PE4
//Pressig the switch will turn on the LED
// Positive Logic is used to interface the switch which measn the switch is connected to the ground by default, and pressing the switch will connect it to 3.3 volt.
// Please look at example_SwitchLED.pdf for more infromation about the circuit.

#define SYSCTL_RCGC2_R          (*((volatile unsigned long *)0x400FE108))
#define GPIO_PORTE_DATA_R       (*((volatile unsigned long *)0x400243FC))
#define GPIO_PORTE_DIR_R        (*((volatile unsigned long *)0x40024400))
#define GPIO_PORTE_AFSEL_R      (*((volatile unsigned long *)0x40024420))
#define GPIO_PORTE_DEN_R        (*((volatile unsigned long *)0x4002451C))
#define GPIO_PORTE_AMSEL_R      (*((volatile unsigned long *)0x40024528))
#define GPIO_PORTE_PCTL_R       (*((volatile unsigned long *)0x4002452C))
	
unsigned long in,in2,out;
void Delay(int secondsMultiplier);

int main(void){ unsigned long volatile delay;
	
	SYSCTL_RCGC2_R |= 0x10;           // Port E clock
  delay = SYSCTL_RCGC2_R;           // wait 3-5 bus cycles
  GPIO_PORTE_DIR_R |= 0x0E;         // PE1,PE2, PE3 output
  GPIO_PORTE_AFSEL_R &= ~0x17;      // not alternative
  GPIO_PORTE_AMSEL_R &= ~0x17;      // no analog
  GPIO_PORTE_PCTL_R = 0x00000000; // bits for PE4,PE2,PE1,PE0
  GPIO_PORTE_DEN_R |= 0x1F;         // enable PE4,PE2,PE1,PE0
	
  while(1){

		in = (GPIO_PORTE_DATA_R&0x01); // in 0 if not pressed, 1 if pressed (Postivice logic has been used this time which means the switch is 1 if pressed)

		in2 = (GPIO_PORTE_DATA_R&0x10);
			
		GPIO_PORTE_DATA_R |= 0x08;
		
		if(in != 0)
		{
			Delay(30); // 3 seconds
			GPIO_PORTE_DATA_R &= 0xF7; // turn off green
			GPIO_PORTE_DATA_R |= 0x04; // turn on yellow
			Delay(40); // 4 seconds
			GPIO_PORTE_DATA_R &= 0xFB; //turn off yellow
			
			while(1)
			{
				in2  = (GPIO_PORTE_DATA_R&0x10);
				GPIO_PORTE_DATA_R |= 0x02; //turn on red
				if(in2!= 0) // check if switch 2 is clicked.
				{
					Delay(40);
					break;
				}
			}
			
			GPIO_PORTE_DATA_R &= 0xFD; //turn off red		
		}
	}
}

void Delay(int secondsMultiplier){
	unsigned long volatile time;
  time = 145448 * secondsMultiplier;  // 0.1sec
  while(time){
		time--;
  }
}
