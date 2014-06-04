//main code
//test code for pwm and motors on msp430 tank robot


//	Header Files

#include <msp430.h> //include msp430 stuff
#include <stdlib.h>
//#include <math.h>
//	Preprocessor Definitions

#define MOTOR_R_A BIT7
#define MOTOR_R_B BIT6		//P1.6 has TA0.1 output

#define MOTOR_L_A BIT3		//note changes here!
#define MOTOR_L_B BIT4		//now on P1.4 for TA0.2 output

#define MOTOR_L_AB ( MOTOR_L_A | MOTOR_L_B)
#define MOTOR_R_AB ( MOTOR_R_A | MOTOR_R_B)

#define MOTOR_LR_PWM ( MOTOR_L_B | MOTOR_R_B )

#define ALL_MOTORS ( MOTOR_L_A | MOTOR_L_B | MOTOR_R_A | MOTOR_R_B )

#define PWM_TOP 1000 //for PWM DIV so DCO/PWM_TOP

//	Global Variables
//volatile unsigned int number_milliseconds=0;
//volatile unsigned char move_direction=0;

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
        
    //_BIS_SR(LPM0_bits + GIE);       // Enter LPM0 and enable interrupts
	
	
	//infinite loop
	for(;;) {
		/*go_forwards(1000); //100%
		bad_delay_ms(2000);
		skid_right(1000);
		bad_delay_ms(1000);
		go_forwards(750); //75%
		bad_delay_ms(2000);
		skid_right(1000);
		bad_delay_ms(1000);
		go_forwards(500); //50%
		bad_delay_ms(2000);
		skid_right(1000);
		bad_delay_ms(1000);
		go_forwards(250); //25%
		bad_delay_ms(2000);
		skid_right(1000);
		bad_delay_ms(1000);*/
		
		for (i = 0; i < 1000;i+=200){
			
			//TACCR1 = i;
			//TACCR2 = i;
			//set_motor_speed(0,i);
			//set_motor_speed(1,i);
			set_motor_speeds(i,i);
			bad_delay_ms(2000);
			//__delay_cycles(100000);
		}
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
	
	//if ( speed == 0 ){
		/*switch (lr) {
			
			case 0:
			P1OUT &= ~MOTOR_L_AB;
				break;
			case 1:
			P1OUT &= ~MOTOR_R_AB;
				break;
		}*/
	//}
	/*else*/ if ( speed > 0 ){
		outspeed = PWM_TOP - speed;
		
		P1OUT |= motor_a_pins[lr];
		/*switch (lr) {
			
			case 0:
			P1OUT |= MOTOR_L_A;
			TACCR2 = outspeed;
				break;
			case 1:
			P1OUT |= MOTOR_R_A;
			TACCR1 = outspeed;
				break;
		}*/
	}
	else if ( speed <= 0 ){
		outspeed = abs(speed);
		
		P1OUT &= ~(motor_a_pins[lr]);
		/*switch (lr) {
			
			case 0:
			P1OUT &= ~MOTOR_L_A;
			TACCR2 = outspeed;
				break;
			case 1:
			P1OUT &= ~MOTOR_R_A;
			TACCR1 = outspeed;
		}*/
	}
	
	switch (lr){ //sets which PWM pin to set
		case 0:
		TACCR2 = outspeed;
			break;
		case 1:
		TACCR1 = outspeed;
			break;
	}
	/*if(lr == 0){
		TACCR2 = outspeed;
	}
	else if(lr == 1){
		TACCR1 = outspeed;
	}*/
	
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

//	Interrupt Service Routines
/*
__attribute__((interrupt(TIMER0_A0_VECTOR))) //notice! for the 20pin chips:
                                             //had to change TIMERA0_VECTOR
                                             //to TIMER0_A0_VECTOR 
void CCR0_ISR(void){
        number_milliseconds++;
       
        if (number_milliseconds == 1000){
			move_direction ^= 1; //toggle direction
			number_milliseconds = 0;
			
			if (move_direction == 0){
				go_forwards();
			} else {
				go_reverse();
			}
		}
}
*/
