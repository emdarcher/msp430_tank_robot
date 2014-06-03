//main code
//test code 2 for msp430 tank robot


//	Header Files

#include <msp430.h> //include msp430 stuff

//	Preprocessor Definitions

#define MOTOR_R_A BIT7
#define MOTOR_R_B BIT6

#define MOTOR_L_A BIT1
#define MOTOR_L_B BIT2

#define ALL_MOTORS ( MOTOR_L_A | MOTOR_L_B | MOTOR_R_A | MOTOR_R_B )

//	Global Variables
volatile unsigned int number_milliseconds=0;
volatile unsigned char move_direction=0;

//	Function Prototypes/Definitions
void go_forwards(void);
void go_reverse(void);
void spin_right(void);
void spin_left(void);
void skid_right(void);
void skid_left(void);
void stop_motors(void);

void motor_l_fwd(void);
void motor_l_rev(void);
void motor_r_fwd(void);
void motor_r_rev(void);

/*
#define MOTOR_L_FWD (MOTOR_L_A & ~(MOTOR_L_B)) 
#define MOTOR_L_REV	(MOTOR_L_B & ~(MOTOR_L_A))
#define MOTOR_R_FWD	
#define MOTOR_R_REV
*/

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
	
	//BCSCTL1 = CALBC1_1MHZ;          // Running at 1 MHz
    //DCOCTL = CALDCO_1MHZ;

	//TACCR0 = 124;           // With the Timer using SMCLK div 8 (125 kHz), this
                              // value gives a frequency of 125000/(TACCR0+1) Hz.
                              // For TACCR0 = 144, that's 862 Hz.
                              // at 14400 it is 8.7... Hz
				//i am getting it to 1kHz
	
	//TACCTL0 = CCIE;         // Enable interrupts for CCR0.
    //TACTL = TASSEL_2 + ID_3 + MC_1 + TACLR;  // SMCLK, div 8, up mode,
                                                                                         // clear timer.
        
    //_BIS_SR(LPM0_bits + GIE);       // Enter LPM0 and enable interrupts
	
	int i;
	//infinite loop
	for(;;) {
		go_forwards();
		bad_delay_ms(1000);
		skid_right();
		bad_delay_ms(500);
		
		/*
		for(i =0;i<10;i++){ 
		__delay_cycles(100000); //really inefficient!
		__delay_cycles(100000);	//just for a rough test though.
		__delay_cycles(100000); //but still really ugly
		__delay_cycles(100000);
		__delay_cycles(100000);
		__delay_cycles(100000);
		__delay_cycles(100000);
		}*/
		//go_reverse();
		/*
		for(i=0;i<10;i++){
		__delay_cycles(100000);
		__delay_cycles(100000);
		__delay_cycles(100000);
		__delay_cycles(100000);
		__delay_cycles(100000);
		__delay_cycles(100000);
		__delay_cycles(100000);
		}*/
	}
	
}

//	Functions

void stop_motors(void){
	//P1OUT &= ~(MOTOR_L_A | MOTOR_L_B | MOTOR_R_A | MOTOR_R_B);
	P1OUT &= ~ALL_MOTORS;
}

void motor_l_fwd(void){
	P1OUT |= MOTOR_L_A;
	P1OUT &= ~MOTOR_L_B;
}
void motor_l_rev(void){
	P1OUT |= MOTOR_L_B;
	P1OUT &= ~MOTOR_L_A;
}
void motor_r_fwd(void){
	P1OUT |= MOTOR_R_A;
	P1OUT &= ~MOTOR_R_B;
}
void motor_r_rev(void){
	P1OUT |= MOTOR_R_B;
	P1OUT &= ~MOTOR_R_A;
}

void go_forwards(void){
	P1OUT |= (MOTOR_L_A | MOTOR_R_A);
	P1OUT &= ~(MOTOR_L_B | MOTOR_R_B);
}
void go_reverse(void){
	P1OUT |= (MOTOR_L_B | MOTOR_R_B);
	P1OUT &= ~(MOTOR_L_A | MOTOR_R_A);
}

void skid_left(void){
	P1OUT |= (MOTOR_R_A);
	P1OUT &= ~(MOTOR_L_A | MOTOR_L_B | MOTOR_R_B);
}
void skid_right(void){
	P1OUT |= (MOTOR_L_A);
	P1OUT &= ~(MOTOR_L_B | MOTOR_R_A | MOTOR_R_B);
}
void spin_left(void){
	P1OUT |= (MOTOR_L_B | MOTOR_R_A);
	P1OUT &= ~(MOTOR_L_A | MOTOR_R_B);
}
void spin_right(void){
	P1OUT |= (MOTOR_L_A | MOTOR_R_B);
	P1OUT &= ~(MOTOR_L_B | MOTOR_R_A);
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
