#include <LPC23xx.H> // LPC2378 definitions
#include <stdint.h>

#define PWM_PERIOD 20000 // Period for 50Hz PWM signal (in microseconds)
#define MIN_PULSE_WIDTH 1000  // Minimum pulse width for 0 degrees (in microseconds)
#define MAX_PULSE_WIDTH 2000 // Maximum pulse width for 180 degrees (in microseconds)

void pwm_init(void);
void set_servo_angle(uint8_t angle);
void delay_ms(unsigned int ms);

int main(void) {

    // Initialize PWM
    pwm_init();

    while (1) {
        set_servo_angle(120);
        delay_ms(500);
				set_servo_angle(60);
        delay_ms(500);
				
    }
}

// Initialize PWM for the servo motor
void pwm_init(void) {
    // Configure PINSEL for PWM1 (P2.0 as PWM1.1)
    PINSEL4 |= (1 << 0); // Set P2.0 to PWM1.1
    PINSEL4 &= ~(1 << 1);

    // Set PWM period (50Hz frequency)
    PWM1PR = 0;                // No prescaler
    PWM1MR0 = PWM_PERIOD;      // Set period in microseconds
    PWM1MCR = (1 << 1);        // Reset on MR0 match

    // Enable PWM1.1
    PWM1PCR = (1 << 9);        // Enable PWM output for PWM1.1
    PWM1LER = (1 << 0) | (1 << 1); // Latch enable for MR0 and MR1

    // Start PWM
    PWM1TCR = (1 << 0) | (1 << 3); // Enable counter and PWM mode
}

// Set the servo angle (0° to 180°)
void set_servo_angle(uint8_t angle) {
    uint32_t pulse_width; // Declare variable at the top of the block

    if (angle > 120) angle = 120; // Limit angle to 180°
    pulse_width = MIN_PULSE_WIDTH + ((MAX_PULSE_WIDTH - MIN_PULSE_WIDTH) * angle) / 120;
    PWM1MR1 = pulse_width; // Set the pulse width for PWM1.1
    PWM1LER = (1 << 1);    // Latch enable for MR1
}

// Simple delay function
void delay_ms(unsigned int ms) {
    unsigned int i; // Declare variable at the top of the block
    for (i = 0; i < ms * 1000; i++) {
        _asm_("nop");
    }
}


