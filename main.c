//infrared sensors for obstacle following robot, disconnect the UART jumpers if you're using P1.1. and P1.2
//observe the port registers P1DIR and P1IN

#include <msp430.h> 
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

// volatile int sensorout; // no longer used

void straight (void);                            // function to move straight
void turnright (void);                           // function to turn right
void turnleft (void);                            // function to turn left
void stop (void);                                // function to stop moving
void set_direction (float left, float right);    // function to set direction

// Forward sensor pins
#define TRIG_PIN1 BIT2    // P1.2
#define ECHO_PIN1 BIT3    // P1.3
#define LED_PIN1 BIT0     // P2.0

// Right sensor pins
#define TRIG_PIN_R BIT4   // P1.4
#define ECHO_PIN_R BIT5   // P1.5
#define LED_PIN_R BIT1    // P2.1

// Left sensor pins
#define TRIG_PIN_L BIT0   // P1.0
#define ECHO_PIN_L BIT1   // P1.1
#define LED_PIN_L BIT2    // P2.2

//Car Motor Control Pins
#define ENABLE_L BIT4     // P2.4
#define ENABLE_R BIT6     // P1.6
#define DRIVE_L BIT3     // P2.3
#define DRIVE_R BIT5     // P2.5

volatile unsigned long start_time1 = 0;    // record start time of front sensor
volatile unsigned long end_time1 = 0;      // record end time of front sensor
volatile unsigned long start_time_R = 0;   // record start time of right sensor
volatile unsigned long end_time_R = 0;     // record end time of right sensor
volatile unsigned long start_time_L = 0;   // record start time of left sensor
volatile unsigned long end_time_L = 0;     // record end time of left sensor

// Flags to send to wheel motors
extern volatile int forward = 0;        // Flag to move forward
extern volatile int forward_f = 0;      // Flag to move forward at increased speed
extern volatile int right = 0;          // Flag to turn right
extern volatile int left = 0;           // Flag to turn left
extern volatile int brake_lights = 0;   // Flag to stop and engage brake lights

float D1; //duty cycle for right wheel
float D2; //duty cycle for left wheel

int period = 8000; //0x0FFF;  //PWM period


void setup() {

    WDTCTL = WDTPW + WDTHOLD;       // Stop watchdog timer

    BCSCTL1 = CALBC1_1MHZ;          // Set DCO to 1MHz
    DCOCTL = CALDCO_1MHZ;


    P1DIR |= TRIG_PIN1 + TRIG_PIN_R + TRIG_PIN_L + ENABLE_R;  // Set trigger and motor enable pins as output
    P1DIR &= ~(ECHO_PIN1 + ECHO_PIN_R + ECHO_PIN_L);          // Clear echo pins as input
    P1OUT &= ~(TRIG_PIN1 + TRIG_PIN_R + TRIG_PIN_L);          // Initialize trigger pins to low
    P1SEL |= ENABLE_R;                                        // Select function for right enable pin

    P2DIR |= LED_PIN1 + LED_PIN_R + LED_PIN_L + ENABLE_L + DRIVE_L + DRIVE_R;  // Set LED pins, motor enable, motor driver pins as output
    P2OUT &= ~(LED_PIN1 + LED_PIN_R + LED_PIN_L);                              // Initialize LED pins as low
    P2SEL |= ENABLE_L;                                                         // Select function for left enable pin

    TACTL = TASSEL_2 + MC_2;      // SMCLK, continuous mode
}

void triggerSensor_R() {            // Trigger signal for the right sensor
    P1OUT |= TRIG_PIN_R;            // Send out trigger to sensor
    __delay_cycles(10);             // 10us delay
    P1OUT &= ~TRIG_PIN_R;           // Stop trigger to sensor
}

void triggerSensor1() {             // Trigger signal for the front sensor
    P1OUT |= TRIG_PIN1;
    __delay_cycles(10);             // 10us delay
    P1OUT &= ~TRIG_PIN1;
}

void triggerSensor_L() {            // Trigger signal for the left sensor
    P1OUT |= TRIG_PIN_L;
    __delay_cycles(10);             // 10us delay
    P1OUT &= ~TRIG_PIN_L;
}

unsigned int measureDistance1() {   // Function to measure distance of front sensor
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

void drive_car(){                    // Function to move car

    if (forward){                    // IF forward flag is set
        set_direction(0.9, 0.9);     // Set duty cycles to control Frenbot speed
    } else if (forward_f){           // IF forward_f flag is set
        set_direction(0.99, 0.99);   // Same as forward but higher duty cycle (i.e. faster)
    } else if (right) {              // If turn flag is set
       set_direction(0.56, 0.99);    // Set different duty cycles to induce turning
    } else if (left) {
        set_direction(0.99, 0.56);
    } else if (brake_lights) {       // IF brake flag is set
        set_direction(0.01, 0.01);   // Set duty cycles to stop
    } else {
        set_direction(0.01, 0.01);
    }

    straight();        // Set motor drive pins
}

unsigned int distance_R = 20;
unsigned int distance_L = 20;

int main(void) {
    setup(); // pin initializations

    while(1) { // indefinite loop


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
            if ((distance_R <= 5)&&(distance_L <= 5)) { // IF both right/left sensors detect something
                P2OUT &= ~LED_PIN_L;                    // no turning is initiated
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
            P2OUT &= ~LED_PIN1;                         // clear all flags and LEDs
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


void set_direction(float left, float right){ // Function to control speed and direction of 2 DC motors using PWM
    D1 = right;    // duty cycle for right motor
    D2 = left;     // duty cycle for left motor

  
    TACCR0 = period-1;  //PWM period
    TACCR1 = period*D2; // Set duty cycle

    TACCTL1 = OUTMOD_7;  //CCR1 selection reset-set
    TACTL = TASSEL_2|MC_1;   //SMCLK submain clock,upmode

    TA1CCR0 = period-1;
    TA1CCR2 = period*D1;

    TA1CCTL2 = OUTMOD_7;  //CCR1 selection reset-set
    TA1CTL = TASSEL_2|MC_1;   //SMCLK submain clock,upmode


}

// Below are functions to set output pins for motor drive pins and associated indicator LEDs
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
