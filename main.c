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

// Forward sensor pins
#define TRIG_PIN1 BIT2    // P1.2
#define ECHO_PIN1 BIT3    // P1.3
#define LED_PIN1 BIT0     //P2.0

// Right sensor pins
#define TRIG_PIN_R BIT4   // P1.4
#define ECHO_PIN_R BIT5   // P1.5
#define LED_PIN_R BIT1    //P2.1

// Left sensor pins
#define TRIG_PIN_L BIT0   // P1.0
#define ECHO_PIN_L BIT1   // P1.1
#define LED_PIN_L BIT2    //P2.2

//Car Control Pins
#define ENABLE_L BIT4 //2.4
#define ENABLE_R BIT6 //1.6
#define DRIVE_L BIT3 //2.3
#define DRIVE_R BIT5 //2.5

volatile unsigned long start_time1 = 0;
volatile unsigned long end_time1 = 0;
volatile unsigned long start_time_R = 0;
volatile unsigned long end_time_R = 0;
volatile unsigned long start_time_L = 0;
volatile unsigned long end_time_L = 0;

// FLAGS TO SEND TO DRIVE MOTORS
extern volatile int forward = 0;        // Flag to move forward
extern volatile int forward_f = 0;      // Flag to move forward at increased speed
extern volatile int right = 0;          // Flag to turn right
extern volatile int left = 0;           // Flag to turn left
extern volatile int brake_lights = 0;   // Flag to engage brake lights (Red LED array)
                                        // brake_lights flag assumes code controlling them are somewhere else; otherwise, set LED pins similarly to the LED_PINs here

float D1; //duty cycle right wheel
float D2; //duty cycle left wheel

int period = 8000; //0x0FFF;  //PWM period


void setup() {

    WDTCTL = WDTPW + WDTHOLD;       // Stop watchdog timer

    BCSCTL1 = CALBC1_1MHZ;          // Set DCO to 1MHz
    DCOCTL = CALDCO_1MHZ;


    P1DIR |= TRIG_PIN1 + TRIG_PIN_R + TRIG_PIN_L + ENABLE_R;
    P1DIR &= ~(ECHO_PIN1 + ECHO_PIN_R + ECHO_PIN_L);
    P1OUT &= ~(TRIG_PIN1 + TRIG_PIN_R + TRIG_PIN_L);
    P1SEL |= ENABLE_R;

    P2DIR |= LED_PIN1 + LED_PIN_R + LED_PIN_L + ENABLE_L + DRIVE_L + DRIVE_R;             // Set LED pin as output
    P2OUT &= ~(LED_PIN1 + LED_PIN_R + LED_PIN_L);            // Set LED pin low
    P2SEL |= ENABLE_L;

    TACTL = TASSEL_2 + MC_2;      // SMCLK, continuous mode
}

void triggerSensor_R() {            // Trigger signal for the right sensor
    P1OUT |= TRIG_PIN_R;
    __delay_cycles(10);             // 10us delay
    P1OUT &= ~TRIG_PIN_R;
}

void triggerSensor1() {             // Trigger signal for the front sensor
    P1OUT |= TRIG_PIN1;
    __delay_cycles(10);          // 10us delay
    P1OUT &= ~TRIG_PIN1;
}

void triggerSensor_L() {            // Trigger signal for the left sensor
    P1OUT |= TRIG_PIN_L;
    __delay_cycles(10);          // 10us delay
    P1OUT &= ~TRIG_PIN_L;
}

unsigned int measureDistance1() {   // Measure distance of front sensor
    //unsigned long pulseWidth;

    while (!(P1IN & ECHO_PIN1));                        // While echo is NOT high
    start_time1 = TAR;                                  // measure start of time interval
    while ((P1IN & ECHO_PIN1));                         // While echo is high
    end_time1 = TAR;                                    // measure end of time interval
    unsigned long duration = end_time1 - start_time1;   // calculate

    // Convert pulse width to distance in cm
    return duration / 58;           // formula given by data-sheet
}

unsigned int measureDistance_R() {  // similar but for right sensor

    while (!(P1IN & ECHO_PIN_R));   // While echo is NOT high
    start_time_R = TAR;
    while ((P1IN & ECHO_PIN_R));      // While echo is high
    end_time_R = TAR;

    unsigned long duration_R = end_time_R - start_time_R;

    return duration_R / 58;
}

unsigned int measureDistance_L() { // similar but for left sensor

    while (!(P1IN & ECHO_PIN_L));   // While echo is NOT high
    start_time_L = TAR;
    while ((P1IN & ECHO_PIN_L));      // While echo is high
    end_time_L = TAR;

    unsigned long duration_L = end_time_L - start_time_L;

    return duration_L / 58;
}

void drive_car(){

    if (forward){
        set_direction(0.9, 0.9);
    } else if (forward_f){
        set_direction(0.99, 0.99);
    } else if (right) {
       set_direction(0.56, 0.99);
    } else if (left) {
        set_direction(0.99, 0.56);
    } else if (brake_lights) {
        set_direction(0.01, 0.01);
    } else {
        set_direction(0.01, 0.01);
    }

    straight();
}

unsigned int distance_R = 20;
unsigned int distance_L = 20;

int main(void) {
    setup();

    while(1) {


        triggerSensor_R();                              // trigger the sensor
        unsigned int distance_R = measureDistance_R();  // check right sensor distance
        __delay_cycles(5000);   // 5ms delay


        triggerSensor1();                               // trigger the middle sensor
        unsigned int distance_1 = measureDistance1();   // check front sensor distance
        __delay_cycles(5000);   // 5ms delay


        triggerSensor_L();                              // trigger the left sensor
        unsigned int distance_L = measureDistance_L();  // check left sensor distance


        if ((distance_R <= 5)||(distance_L <= 5)) {     // IF either right or left sensor detected (i.e. car wants to turn)
            P2OUT &= ~LED_PIN1;                         // forward flag is cleared
            forward = 0;
            if ((distance_R <= 5)&&(distance_L <= 5)) {
                P2OUT &= ~LED_PIN_L;
                P2OUT &= ~LED_PIN_R;
                left = 0;
                right = 0;
            }
            else {
                if (distance_R <= 5) {              // IF right sensor <= 5cm
                    P2OUT |= LED_PIN_R;             // right flag is set
                    right = 1;
                }
                else {
                    P2OUT &= ~LED_PIN_R;            // right flag is cleared
                    right = 0;
                }
                if (distance_L <= 5) {              // IF left sensor <= 5cm
                    P2OUT |= LED_PIN_L;             // left flag is set
                    left = 1;
                }
                else {
                    P2OUT &= ~LED_PIN_L;            // left flag is cleared
                    left = 0;
                }
            }
        }

        else if (distance_1 <= 20) {                    // IF front sensor <= 20cm, move forward
            P2OUT &= ~LED_PIN_R;
            P2OUT &= ~LED_PIN_L;
            left = 0;   // clear turn flags
            right = 0;

            if (distance_1 <= 3) {                      // IF front sensor <= 3cm, stop and brake
                brake_lights = 1;   // set brake lights flag
                forward_f = 0;      // clear faster forward flag
                forward = 0;        // clear normal forward flag
                P2OUT &= ~LED_PIN1;
            } else if (distance_1 <= 10){               // IF front sensor between 4-10cm, move forward at normal speed
                brake_lights = 0;   // clear brake lights flag
                forward_f = 0;      // clear faster forward flag
                forward = 1;        // set normal forward flag
                P2OUT |= LED_PIN1;

            } else {                                    // IF front sensor between 10-20cm, move forward at faster speed
                brake_lights = 0;   // clear brake lights flag
                forward_f = 1;      // set faster forward flag (car moves faster when hand further away)
                forward = 0;        // clear normal forward flag
                P2OUT |= LED_PIN1;
            }


        }
        else {  // Idle state
            P2OUT &= ~LED_PIN1;                         // clear all flags
            P2OUT &= ~LED_PIN_R;
            P2OUT &= ~LED_PIN_L;
            forward = 0;
            right = 0;
            left = 0;
            brake_lights = 0;

        }

        drive_car();

        __delay_cycles(100000);    // Delay between measurements (0.1 secs)
    }

}


void set_direction(float left, float right){
    D1 = right;
    D2 = left;

    TACCR0 = period-1;  //PWM period
    TACCR1 = period*D2;

    TACCTL1 = OUTMOD_7;  //CCR1 selection reset-set
    TACTL = TASSEL_2|MC_1;   //SMCLK submain clock,upmode

    TA1CCR0 = period-1;
    TA1CCR2 = period*D1;

    TA1CCTL2 = OUTMOD_7;  //CCR1 selection reset-set
    TA1CTL = TASSEL_2|MC_1;   //SMCLK submain clock,upmode


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
