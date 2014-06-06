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

void ADC_read_vals(void);

int analog_to_pwm(unsigned int analog);

void set_pwms(void);

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
	
	ADC_init();
	
	//infinite loop
	for(;;) {
		/*__delay_cycles(PWM_TOP*5);
		TACCR1 += dir1;
		TACCR2 += dir2;
		if (TACCR1 == PWM_TOP){
			dir1 = -1; //switch directions
			dir2 = 1;
		} else if (TACCR1 == 0){
			dir1 = 1;
			dir2 = -1;
		}*/
		
		ADC_read_vals();
		set_pwms();
		
		
	}
	//return 0; //should never reach this	
}

//	Functions

void set_pwms(void){
	
	TACCR1 = analog_to_pwm(a0_val);
	TACCR2 = analog_to_pwm(a1_val);	
		
}

int analog_to_pwm(unsigned int analog){
	
	if( analog > 1000 ){
		
		return (analog - 23);
		
	} else {
		return analog;
	}
	
}

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
	ADC10CTL0 = ADC10SHT_1 + MSC + ADC10ON + ADC10IE + SREF_0;
	// ADC10CTL0 control Register 0
	// ADC10SHT_1 sample and hold time, 1 = 8 x ADC10CLK
	// MSC multiple sample and conversions
	// ADC10ON adc10 on
	// ADC10IE adc10 interrupt enable
	// SREF_0 use external V ref, so Vcc/Vss
	ADC10DTC1 = 0x02;
	// Data Transfer Control register, defines the number of transfers in each block, 2 in this case
	ADC10AE0 |= (A0 | A1);
	// ADC10 analog enable bits PP1.1 and P1.0 
}

void ADC_read_vals(void){
	
	ADC10CTL0 &= ~ENC; //disable conversion
	while (ADC10CTL1 & BUSY);               // Wait if ADC10 core is active
	ADC10SA = (unsigned int)analog_vals;			// Copies data in ADC10SA to unsigned int adc array
    ADC10CTL0 |= ENC + ADC10SC;             // Sampling and conversion start
	__bis_SR_register(CPUOFF + GIE);        // LPM0, ADC10_ISR will force exit
	
	a0_val = analog_vals[1];
	a1_val = analog_vals[0];
	
}

//	Interrupt Service Routines

// ADC10 interrupt service routine
/*#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void)
{
  __bic_SR_register_on_exit(CPUOFF);        // Clear CPUOFF bit from 0(SR)
}*/
__attribute__((interrupt(ADC10_VECTOR)))
void ADC10_ISR(void){
	
	__bic_SR_register_on_exit(CPUOFF);        // Clear CPUOFF bit from 0(SR)
	
}
