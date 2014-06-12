//main code
//test code for pwm, LDRs, and motors on msp430 tank robot
//the output from the two LDR is compared to find which one is greater and 
//the difference between their values.
//The difference is then applied to a difference in pwms from a midpoint
//to the motors, making it turn more intensly if the difference between
//the light values is greater and vise versa.
//
//this method should be better because it works on the difference not the 
//direct values, so it will work fine in a wide range of ambient light.
//also seems more natural.

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
#define FULL_PWM PWM_TOP
#define MID_PWM (FULL_PWM/2)



//	Global Variables

unsigned int analog_vals[2]={0,0};
unsigned int a0_val = 0;
unsigned int a1_val = 0;
unsigned int pwm_vals_lr[2] = {0,0};


const unsigned char motor_a_pins[] = {MOTOR_L_A, MOTOR_R_A};
//const unsigned char motor_ab_pins[] = {MOTOR_L_AB, MOTOR_R_AB};

//int i; //for "for" loops

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
		
		ADC_read_vals(); //read ADC input values
		process_vals(); //process values and set motor pwms accordingly
	}
	
}

//	Functions
void process_vals(unsigned int val1, unsigned int val2 ){
     //processes vals to determine left or right brighter and diff.
    //val1 will be the left and val2 right in this case
    
    //unsigned int diff_vals;
    
    val1 = analog_to_pwm(val1);
    val2 = analog_to_pwm(val2);
    
    if(val1 == val2) {
        set_led_pwms(FULL_PWM,FULL_PWM);
    }
    else if(val1 > val2){
        diff_to_pwms(0x00,(val1-val2));
    }
    else if(val1 < val2){
        diff_to_pwms(0x01,(val2-val1));
    }
            
}

//takes a difference and side then outputs correctly to motors
void diff_to_pwms(unsigned char side, unsigned int difference){
    //side can be 0 or 1, left or right, respectively
    
    unsigned int half_diff = (difference/2);
    //pwm_vals_lr[side] = MID_PWM + half_diff;
    //pwm_vals_lr[!(side)] = MID_PWM - half_diff;
    
    //switched for opposite sides b/c it is turning to light with motors
    pwm_vals_lr[side] = MID_PWM - half_diff;
    pwm_vals_lr[!(side)] = MID_PWM + half_diff;
          
    set_motor_speeds(pwm_vals_lr[0],pwm_vals_lr[1]);
    
}

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
		outspeed = PWM_TOP - speed; //if positive then forward
		P1OUT |= motor_a_pins[lr];
	}
	else if ( speed <= 0 ){  //if a negetive value then reverse
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
	//to convert the analog 10 bit 0-1023
    //to 0-1000, but just by subtracting 32 if over 1000 or PWM_TOP.
    //not directly accurate, but close enough in this case and less math.
	if( analog > PWM_TOP){
		
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
	
	ADC10CTL1 = INCH_1 + CONSEQ_1;
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
	ADC10SA = (unsigned int)&analog_vals[0];			// Copies data in ADC10SA to unsigned int adc array
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
