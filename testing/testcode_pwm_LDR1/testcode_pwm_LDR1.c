//main code
//test code for pwm, LDRs, and motors on msp430 tank robot


//	Header Files

#include <msp430.h> //include msp430 stuff
#include <stdlib.h>

//	Preprocessor Definitions

//motor control pins
#define MOTOR_R_A BIT7
#define MOTOR_R_B BIT6		//P1.6 has TA0.1 output

#define MOTOR_L_A BIT3		//note changes here!
#define MOTOR_L_B BIT4		//now on P1.4 for TA0.2 output

#define MOTOR_L_AB ( MOTOR_L_A | MOTOR_L_B )
#define MOTOR_R_AB ( MOTOR_R_A | MOTOR_R_B )

#define MOTOR_LR_PWM ( MOTOR_L_B | MOTOR_R_B )

#define ALL_MOTORS ( MOTOR_L_A | MOTOR_L_B | MOTOR_R_A | MOTOR_R_B )

//analog input pins
#define A0 BIT0
#define A1 BIT1

#define LDR_L A0 //more friendly names?
#define LDR_R A1 //for the left and right LDRs


//PWM stuff
#define PWM_TOP 1000 //for PWM DIV so DCO/PWM_TOP



//	Global Variables
//volatile unsigned int number_milliseconds=0;
//volatile unsigned char move_direction=0;


unsigned int analog_vals[2];
unsigned int a0_val = 0;
unsigned int a1_val = 0;

const unsigned char motor_a_pins[] = {MOTOR_L_A, MOTOR_R_A};
//const unsigned char motor_ab_pins[] = {MOTOR_L_AB, MOTOR_R_AB};

int i; //for "for" loops

//	Function Prototypes/Definitions
void go_forwards(int spd);
void go_reverse(int spd);
void spin_right(int spd);
void spin_left(int spd);
void skid_right(int spd);
void skid_left(int spd);
void stop_motors(void);

void motor_l_fwd(int spd);
void motor_l_rev(int spd);
void motor_r_fwd(int spd);
void motor_r_rev(int spd);

void set_motor_speed(unsigned char lr, int speed);
void set_motor_speeds(int speed_left, int speed_right);

void TA_init(void);

void ADC_init(void);

void ADC_read_vals(void);

int analog_to_pwm(unsigned int analog);

void bad_delay_ms(unsigned int ms){
	int j;
	for (j=0;j<ms;j++){
		//expects to be operating a 1MHz
		__delay_cycles(1000);
	}
}

void main(void) {
	WDTCTL = WDTPW + WDTHOLD; //disable watchdog
	
	//initialization stuff here
		//remember to enable any pull-up/downs!
	
	P1OUT = 0;
	P1DIR |= ALL_MOTORS;
	P1SEL |= MOTOR_LR_PWM;
	P1SEL2 |= MOTOR_L_B; //P1SEL2 needed as well for TA0.2
	
	BCSCTL1 = CALBC1_1MHZ;          // Running at 1 MHz
    DCOCTL = CALDCO_1MHZ;

	TA_init();
    
    ADC_init();
    
    //_BIS_SR(LPM0_bits + GIE);       // Enter LPM0 and enable interrupts
	
	
	//infinite loop
	for(;;) {
		
		ADC_read_vals();
		set_motor_speeds(analog_to_pwm(a0_val),analog_to_pwm(a1_val));
		
	}
	
}

//	Functions

void set_motor_speeds(int speed_left, int speed_right){
	
	set_motor_speed(0, speed_left);
	set_motor_speed(1, speed_right);
	
}
void set_motor_speed(unsigned char lr, int speed){
	//sets the sets outputs + pwm based on input speed
	//reverses motor for negative speed values
	
	//lr = 0 or 1 for left or right, respectively 
	int outspeed;

if(lr < 2){ //just check to make sure lr value is valid 0 or 1	
	if ( speed > 0 ){
		outspeed = PWM_TOP - speed;
		P1OUT |= motor_a_pins[lr];
	}
	else if ( speed <= 0 ){
		outspeed = abs(speed);
		P1OUT &= ~(motor_a_pins[lr]);
	}
	
	switch (lr){ //sets which PWM pin to set
		case 0:
		TACCR2 = outspeed;
			break;
		case 1:
		TACCR1 = outspeed;
			break;
	}
}

}

void stop_motors(void){
	//P1OUT &= ~(MOTOR_L_A | MOTOR_L_B | MOTOR_R_A | MOTOR_R_B);
	//P1OUT &= ~ALL_MOTORS;
	set_motor_speeds(0,0);
}

void motor_l_fwd(int spd){
	//P1OUT |= MOTOR_L_A;
	//P1OUT &= ~MOTOR_L_B;
	
	set_motor_speed(0,spd);
	
}
void motor_l_rev(int spd){
	//P1OUT |= MOTOR_L_B;
	//P1OUT &= ~MOTOR_L_A;
	spd *= -1;//make negative
	set_motor_speed(0,spd);
}
void motor_r_fwd(int spd){
	//P1OUT |= MOTOR_R_A;
	//P1OUT &= ~MOTOR_R_B;
	set_motor_speed(1,spd);
}
void motor_r_rev(int spd){
	//P1OUT |= MOTOR_R_B;
	//P1OUT &= ~MOTOR_R_A;
	spd *= -1;//make negative
	set_motor_speed(1,spd);
}

void go_forwards(int spd){
	//P1OUT |= (MOTOR_L_A | MOTOR_R_A);
	//P1OUT &= ~(MOTOR_L_B | MOTOR_R_B);
	
	set_motor_speeds(spd,spd);
}
void go_reverse(int spd){
	//P1OUT |= (MOTOR_L_B | MOTOR_R_B);
	//P1OUT &= ~(MOTOR_L_A | MOTOR_R_A);
	spd *= -1;//make negative
	set_motor_speeds(spd,spd);
}

void skid_left(int spd){
	//P1OUT |= (MOTOR_R_A);
	//P1OUT &= ~(MOTOR_L_A | MOTOR_L_B | MOTOR_R_B);
	set_motor_speeds(0,spd);
}
void skid_right(int spd){
	//P1OUT |= (MOTOR_L_A);
	//P1OUT &= ~(MOTOR_L_B | MOTOR_R_A | MOTOR_R_B);
	set_motor_speeds(spd,0);
}
void spin_left(int spd){
	//P1OUT |= (MOTOR_L_B | MOTOR_R_A);
	//P1OUT &= ~(MOTOR_L_A | MOTOR_R_B);
	set_motor_speeds((-1*spd),spd);
}
void spin_right(int spd){
	//P1OUT |= (MOTOR_L_A | MOTOR_R_B);
	//P1OUT &= ~(MOTOR_L_B | MOTOR_R_A);
	set_motor_speeds(spd,(-1*spd));
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
	TACCR1 = 0;				// 0.0% duty cycle for PWMTop = 1000
	TACTL = TASSEL_2 + ID_0 + MC_1;	// SMCLK, div 1, Up Mode
									// No interrupts needed!
	TACCTL1 = OUTMOD_7;		// Reset/Set: Sets at TACCR0, resets at TACCR1
	
	//stuff for TA0.2
	TACCR2 = 0; //0.0%
	TACCTL2 = OUTMOD_7;
	
}

void ADC_init(void){
	
	ADC10CTL1 = INCH_2 + CONSEQ_1;
	// ADC10CTL1 control register 1, channel 1 highest conversion channel
    //make sure INCH_X val is correct!!!!
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
	//original way, but a little less nice lookin'
	//__bis_SR_register(CPUOFF + GIE);        // LPM0, ADC10_ISR will force exit
	
	_BIS_SR(LPM0_bits + GIE);       // Enter LPM0 and enable interrupts
	
	a0_val = analog_vals[1];
	a1_val = analog_vals[0];
	
}

//	Interrupt Service Routines

__attribute__((interrupt(ADC10_VECTOR)))
void ADC10_ISR(void){
	
	//__bic_SR_register_on_exit(CPUOFF);        // Clear CPUOFF bit from 0(SR)
	//
	//
	_BIC_SR_IRQ(LPM0_bits); //following convention
	
	//simpler way:
	//LPM0_EXIT;
}
