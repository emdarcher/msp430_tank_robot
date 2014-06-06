//main code
//to test two ADC inputs and outputing corrsponding PWM
//to two LEDs


//	Header Files

#include <msp430.h> //include msp430 stuff

//	Preprocessor Definitions

#define LED1 BIT4
#define LED2 BIT6

#define ALL_LEDS (LED1 | LED2)

#define PWM_TOP	1000	// Set PWM Frequency: DCO/PWM_TOP

//analog stuff
#define A0 BIT0
#define A1 BIT1

//	Global Variables

unsigned int analog_vals[2];
unsigned int a0_val = 0;
unsigned int a1_val = 0;

//	Function Prototypes/Definitions

void TA_init(void);

void ADC_init(void);

void main(void) {
	WDTCTL = WDTPW + WDTHOLD; //disable watchdog
	
	int dir1=1;		// Direction for PWM duty cycle change. A +1
					// value increases, a -1 value decreases
	int dir2=-1;    //start direction for number 2
	
	//initialization stuff here
		//remember to enable any pull-up/downs!
		
	P1OUT = 0;
	P1DIR |= ALL_LEDS;
	P1SEL |= (LED2 | LED1); //initialize the TA0.1 on led2,
					//and TA0.2 on led1
	P1SEL2 |= LED1;
							//P1SEL2 is important here!
	
	BCSCTL1 = CALBC1_1MHZ;          // Running at 1 MHz
    DCOCTL = CALDCO_1MHZ;
    
    TA_init();
	
	//infinite loop
	for(;;) {
		__delay_cycles(PWM_TOP*5);
		TACCR1 += dir1;
		TACCR2 += dir2;
		if (TACCR1 == PWM_TOP){
			dir1 = -1; //switch directions
			dir2 = 1;
		} else if (TACCR1 == 0){
			dir1 = 1;
			dir2 = -1;
		}
		
	}
	//return 0; //should never reach this	
}

//	Functions

void TA_init(void) {
	TACCR0 = PWM_TOP;		// TACCR0 controls the PWM frequency
	TACCR1 = 1;				// LED2 starts at 1% duty cycle for PWMTop = 1000
	TACTL = TASSEL_2 + ID_0 + MC_1;	// SMCLK, div 1, Up Mode
									// No interrupts needed!
	TACCTL1 = OUTMOD_7;		// Reset/Set: Sets at TACCR0, resets at TACCR1
	
	//stuff for TA0.2
	TACCR2 = PWM_TOP; //led1 100% duty cycle
	TACCTL2 = OUTMOD_7;
	
}

void ADC_init(void){
	
	ADC10CTL1 = INCH_2 + CONSEQ_1;
	// ADC10CTL1 control register 1, channel 2 highest conversion channel
	// CONSEQ_1 conversion seq mode 3 repeat sequence of channels
	
}

//	Interrupt Service Routines

