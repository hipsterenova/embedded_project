#include <LPC23xx.h>    // LPC2378 register definitions
#include <stdlib.h>     // For rand() and srand()
#include <stdio.h>      // For sprintf()
#include "lcd.h"        // Include LCD library

// Function Prototypes
void UART1_Init(void);
void send_sms(char *message);
void UART1_SendString(char *str);
void UART1_SendChar(char c);
void GPIO_Init(void);
void delay_us(unsigned int microseconds);
void stepMotor(int step);
void rotateMotorOneStep(void);
int isButtonPressed(void);

// Pin Definitions for Stepper Motor (Connected to P0.1, P0.2, P0.3, P0.4)
#define IN1 (1 << 1)  // P0.1//pin no 67
#define IN2 (1 << 2)  // P0.2//pin no 141
#define IN3 (1 << 3)  // P0.3//pin no 142
#define IN4 (1 << 4)  // P0.4//pin no 116

// Pin Definitions for Button and Buzzer
#define BUTTON_PIN (1 << 10)  // P0.10 (Button)//pin no 69
#define BUZZER_PIN (1 << 11)  // P0.11 (Buzzer)//pin no 70

// Step delay in microseconds
#define STEP_DELAY 1000        // 1 ms delay between steps
#define ONE_MINUTE_DELAY 14488500  // 1-minute delay in microseconds
#define CHECK_BUTTON_DELAY 24148   // Check button every 100 ms
#define TEN_SECONDS_DELAY 2414750  // 10-second delay in microseconds

// Function to initialize GPIO pins
void GPIO_Init(void) {
    IODIR0 |= (IN1 | IN2 | IN3 | IN4 | BUZZER_PIN);  // Set motor and buzzer pins as outputs
    IODIR0 &= ~BUTTON_PIN;  // Set button pin as input
}

// UART1 Initialization for GSM Communication
void UART1_Init(void) {
    PINSEL0 |= (1 << 30);  // Configure P0.15 as TXD1//Pin no 89
    PINSEL1 |= (1 << 0);   // Configure P0.16 as RXD1// Pin no 90
    U1LCR = 0x83;                     // Enable DLAB, 8-bit data, no parity, 1 stop bit
    U1DLL = 97;                       // Set baud rate to 9600 (assuming 12 MHz PCLK)
    U1DLM = 0;
    U1LCR &= ~0x80;                   // Disable DLAB
}

// Function to send a single character via UART1
void UART1_SendChar(char c) {
    while (!(U1LSR & (1 << 5)));  // Wait for UART1 to be ready
    U1THR = c;  // Send character
}

// Function to send a string via UART1
void UART1_SendString(char *str) {
    while (*str) {
        UART1_SendChar(*str);
        str++;
    }
}

// Function for a microsecond delay
void delay_us(unsigned int microseconds) {
    volatile unsigned int count;
    for (count = 0; count < (microseconds * 12); count++) {
        __asm("NOP");  // No operation
    }
}

// Function to control stepper motor sequence
void stepMotor(int step) {
    switch (step) {
        case 0:
            IOSET0 = IN1;
            IOCLR0 = IN2 | IN3 | IN4;
            break;
        case 1:
            IOSET0 = IN2;
            IOCLR0 = IN1 | IN3 | IN4;
            break;
        case 2:
            IOSET0 = IN3;
            IOCLR0 = IN1 | IN2 | IN4;
            break;
        case 3:
            IOSET0 = IN4;
            IOCLR0 = IN1 | IN2 | IN3;
            break;
    }
}

// Function to rotate the motor by one step
void rotateMotorOneStep(void) {
    int i;
    for (i = 0; i < 512; i++) {  // 512 steps for 1/4th of a rotation (adjust for your motor)
        stepMotor(i % 4);        // Execute step
        delay_us(STEP_DELAY);    // Delay between steps
    }
    IOCLR0 = IN1 | IN2 | IN3 | IN4;  // Turn off motor pins
}

// Function to check if button is pressed
int isButtonPressed(void) {
    return (IOPIN0 & BUTTON_PIN) == 0;  // Return true if button is pressed (active low)
}

// Function to send SMS via GSM SIM808 module
void send_sms(char *message) {
    UART1_SendString("AT+CMGF=1\r");  // Set SMS mode
    delay_us(100000);  // Wait for response

    UART1_SendString("AT+CMGS=\"+918660673922\"\r");  // Replace with actual phone number
    delay_us(100000);  // Wait for response

    UART1_SendString(message);  // Send the message
    delay_us(100000);  // Wait for message to be processed

    UART1_SendChar(26);  // Ctrl+Z to send the SMS
    delay_us(100000);  // Wait for message to be sent
}

// Main function
int main(void) {
    GPIO_Init();     // Initialize GPIO pins
    UART1_Init();    // Initialize UART1 for GSM module
    lcd_init();      // Initialize the LCD

    while (1) {
        int i, buttonPressed = 0;

        // Rotate motor and display "Take Medicine"
        rotateMotorOneStep();
        lcd_clear();
        set_cursor(0, 0);
        lcd_print((const unsigned char *)"Take Medicine");

        // Wait for button press for 10 seconds
        for (i = 0; i < TEN_SECONDS_DELAY / CHECK_BUTTON_DELAY; i++) {
            if (isButtonPressed()) {
                buttonPressed = 1;
                break;
            }
            delay_us(CHECK_BUTTON_DELAY);
        }

        if (buttonPressed) {
            lcd_clear();
            set_cursor(0, 0);
            lcd_print((const unsigned char *)"Medicine Taken");
            set_cursor(0, 1);
            lcd_print((const unsigned char *)"Wait for Next");
        } else {
            IOSET0 = BUZZER_PIN;  // Turn on buzzer
            lcd_clear();
            set_cursor(0, 0);
            lcd_print((const unsigned char *)"Take Medicine");

            // Check for button press during buzzer interval
            for (i = 0; i < TEN_SECONDS_DELAY / CHECK_BUTTON_DELAY; i++) {
                if (isButtonPressed()) {
                    buttonPressed = 1;
                    IOCLR0 = BUZZER_PIN;  // Turn off buzzer immediately
                    lcd_clear();
                    set_cursor(0, 0);
                    lcd_print((const unsigned char *)"Medicine Taken");
                    set_cursor(0, 1);
                    lcd_print((const unsigned char *)"Wait for Next");
                    break;
                }
                delay_us(CHECK_BUTTON_DELAY);
            }
            IOCLR0 = BUZZER_PIN;  // Turn off buzzer after second interval
        }
 
        if (!buttonPressed) {
            send_sms("Medicine not taken! Dose Missed");
        }

        delay_us(ONE_MINUTE_DELAY);  // Wait 1 minute before next notification
    }

    return 0;
}
