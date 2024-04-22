//infrared sensors for line following robot, disconnect the UART jumpers if you're using P1.1. and P1.2
//observe the port registers P1DIR and P1IN

#include <msp430.h> 
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

volatile int sensorout;


void straight (void);
void turnright (void);
void turnleft (void);
void stop (void);
void set_direction (float left, float right);


float D1; //duty cycle new shit wheel
float D2; //duty cycle shit wheel

int period = 8000; //0x0FFF;  //PWM period


void main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    P1DIR |= 0xE7; //defining  P1.4 P1.3  as input, 2 sensors, the rest output

    P1SEL |= BIT6; //defining P1.6 as timer (PWM)

    P2DIR = 0xFF;

    P2DIR |= BIT4;
    P2SEL |= BIT4;

    /*

    TACCR0 = period-1;  //PWM period

    //TACCR1 = period*D2;  //CCR1 PWM Duty Cycle

    TACCTL1 = OUTMOD_7;  //CCR1 selection reset-set
    TACTL = TASSEL_2|MC_1;   //SMCLK submain clock,upmode

    TA1CCR0 = period-1;

    //TA1CCR2 = period*D1;

    TA1CCTL2 = OUTMOD_7;  //CCR1 selection reset-set
    TA1CTL = TASSEL_2|MC_1;   //SMCLK submain clock,upmode
*/

    /*
    P1DIR |= BIT6;
    P2DIR |= BIT4;

    P2OUT = 0x00;

    P1OUT |= BIT6;
    P2OUT |= BIT4; //0x24;
    */


    //P2OUT |= BIT3 + BIT5; //0x24;

    /*
    D1 = 0.59;
    D2 = 0.59;
    */

    set_direction(0.2,0.2);

    while (1){

        TACCR0 = period-1;  //PWM period
        TACCR1 = period*D2;

        TACCTL1 = OUTMOD_7;  //CCR1 selection reset-set
        TACTL = TASSEL_2|MC_1;   //SMCLK submain clock,upmode


        TA1CCR0 = period-1;
        TA1CCR2 = period*D1;

        TA1CCTL2 = OUTMOD_7;  //CCR1 selection reset-set
        TA1CTL = TASSEL_2|MC_1;   //SMCLK submain clock,upmode

        straight();
    }

}


void set_direction(float left, float right){
    D1 = right;
    D2 = left;

    /*
    TACCR1 = period*D2;
    TA1CCR2 = period*D1;
    */

}

void straight(void){
    //P2OUT = 0x00;

    P2OUT |= BIT3 + BIT5; //0x24;
}

void straight_back(void){
    //P2OUT = 0x00;
    P2OUT |= BIT2 + BIT1; //0x24;
}

void turnleft(void){
    //P2OUT = 0x00;
    P2OUT |= BIT3 + BIT2 + BIT5;
}

void turnright(void){
    //P2OUT = 0x00;
    P2OUT |= BIT5 + BIT1 + BIT3;
}

void stop(void){
    P2OUT = 0x00;
    P2OUT |= 0x00;
}
