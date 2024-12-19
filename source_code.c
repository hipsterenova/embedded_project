#include <LPC23xx.h>
#include "LCD.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h> // Include for atoi

#define DEBOUNCE_DELAY 1000000
#define MAX_COMPARTMENTS 4
#define MAX_USERS 10

// Keypad mappings
#define COL1 (1 << 1)
#define COL2 (1 << 2)
#define COL3 (1 << 3)
#define COL4 (1 << 4)
#define ROW1 (1 << 5)
#define ROW2 (1 << 6)
#define ROW3 (1 << 7)
#define ROW4 (1 << 8)

void delay(long amt) {
    long i;
    for (i = 0; i < amt; i++); // Simple delay loop
}

void keypad_init(void) {
    FIO2DIR |= (COL1 | COL2 | COL3 | COL4); // Set columns (P2.1 - P2.4) as output
    FIO2DIR &= ~(ROW1 | ROW2 | ROW3 | ROW4); // Set rows (P2.5 - P2.8) as input
    FIO2SET = (COL1 | COL2 | COL3 | COL4); // Set all column outputs high initially
}

void display_message(const char *message) {
    lcd_clear();
    lcd_print((unsigned char *)message);
}

void update_buffer_display(char *buffer) {
    lcd_clear();
    lcd_print((unsigned char *)buffer);
}

char keypad_getkey(void) {
    int col;
    const char keymap[4][4] = {
        {'1', '2', '3', 'A'},
        {'4', '5', '6', 'B'},
        {'7', '8', '9', 'C'},
        {'*', '0', '#', 'D'}
    };
    for (col = 0; col < 4; col++) {
        FIO2SET = (COL1 | COL2 | COL3 | COL4); // Set all high
        FIO2CLR = (1 << (col + 1)); // Clear the current column
        delay(1000000); // Short delay to allow signals to stabilize
        if (!(FIO2PIN & ROW1)) { delay(DEBOUNCE_DELAY); return keymap[0][col]; }
        if (!(FIO2PIN & ROW2)) { delay(DEBOUNCE_DELAY); return keymap[1][col]; }
        if (!(FIO2PIN & ROW3)) { delay(DEBOUNCE_DELAY); return keymap[2][col]; }
        if (!(FIO2PIN & ROW4)) { delay(DEBOUNCE_DELAY); return keymap[3][col]; }
    }
    return '\0'; // No key pressed
}

// Structures for admin and user data
typedef struct {
    int userID;
    int meds[MAX_COMPARTMENTS];
    int meal; // 1: Breakfast, 2: Lunch, 3: Dinner
} User;

User users[MAX_USERS];
int user_count = 0;
int meds[MAX_COMPARTMENTS] = {10, 10, 10, 10};

// Functionality: Add User
void add_user(void) {
		char buffer[16] = {0};
    int buf_index = 0;
    char key;
    User new_user;
		int i;

    if (user_count >= MAX_USERS) {
        display_message("Max users reached");
        delay(2000000);
        return;
    }

    
    display_message("Enter User ID:");
    while (1) {
        key = keypad_getkey();
        if (key == 'A') break; // Confirm ID
        if (key >= '0' && key <= '9' && buf_index < 2) {
            buffer[buf_index++] = key;
            buffer[buf_index] = '\0';
            update_buffer_display(buffer);
        }
    }
    new_user.userID = atoi(buffer);

    // Enter meds for each compartment
		
    for (i = 0; i < MAX_COMPARTMENTS; i++) {
        buf_index = 0;
        memset(buffer, 0, sizeof(buffer));
        sprintf(buffer, "C%d Meds:", i + 1);
        display_message(buffer);

        while (1) {
            key = keypad_getkey();
            if (key == 'A') break; // Confirm meds
            if (key >= '0' && key <= '9') {
                buffer[buf_index++] = key;
                buffer[buf_index] = '\0';
                update_buffer_display(buffer);
            }
        }
        new_user.meds[i] = atoi(buffer);
    }

    // Select meal
    display_message("Meal (1-3):");
    while (1) {
        key = keypad_getkey();
        if (key >= '1' && key <= '3') {
            new_user.meal = key - '0';
            break;
        }
    }

    users[user_count++] = new_user;
    display_message("User Added");
    delay(2000000);
}

// Functionality: Update Meds
void update_meds(void) {
    int selected_compartments[MAX_COMPARTMENTS] = {0};
    char buffer[16] = {0};
    int buf_index = 0;
    char key;
		int i;

    display_message("Select Compartments:");
    while (1) {
        key = keypad_getkey();
        if (key == 'A') break; // Confirm compartments
        if (key >= '1' && key <= '4') {
            int compartment = key - '1';
            if (!selected_compartments[compartment]) {
                selected_compartments[compartment] = 1;
                buffer[buf_index++] = key;
                buffer[buf_index] = '\0';
                update_buffer_display(buffer);
            }
        }
    }

    // Update meds for each selected compartment
    for (i = 0; i < MAX_COMPARTMENTS; i++) {
        if (selected_compartments[i]) {
            buf_index = 0;
            memset(buffer, 0, sizeof(buffer));
            sprintf(buffer, "C%d Add Meds:", i + 1);
            display_message(buffer);

            //int meds_to_add = 0;
            while (1) {
                key = keypad_getkey();
                if (key == 'A') break; // Confirm meds
                if (key >= '0' && key <= '9') {
                    buffer[buf_index++] = key;
                    buffer[buf_index] = '\0';
                    update_buffer_display(buffer);
                }
            }
            meds[i] += atoi(buffer); // Add meds
        }
    }

    display_message("Meds Updated");
    delay(2000000);
}

// Functionality: Display Meds
void display_meds(void) {
    char message[32];
    lcd_clear();
    sprintf(message, "C1:%d C2:%d", meds[0], meds[1]);
    set_cursor(0, 0);
    lcd_print((unsigned char *)message);
    sprintf(message, "C3:%d C4:%d", meds[2], meds[3]);
    set_cursor(0, 1);
    lcd_print((unsigned char *)message);
    delay(3000000);
}

// Functionality: Dispense Meds
void dispense_meds(int userID, int meal) {
		int i; 
    for (i = 0; i < user_count; i++) {
        if (users[i].userID == userID && users[i].meal == meal) {
            char message[32];
						int j; 
						int k;
            for (j = 0; j < MAX_COMPARTMENTS; j++) {
                if (meds[j] < users[i].meds[j]) {
                    sprintf(message, "C%d needs refill", j + 1);
                    display_message(message);
                    return;
                }
            }
						
            for (k = 0; k < MAX_COMPARTMENTS; k++) {
                meds[k] -= users[i].meds[k];
                sprintf(message, "Dispensed C%d", k + 1);
                display_message(message);
                delay(1000000);
            }

            display_message("Dispense Done");
            return;
        }
    }
    display_message("User/Meal Invalid");
}

int main(void) {
    char key;
    char mode = 'A'; // Start in Admin mode
    char buffer[16] = {0};
    int buf_index = 0;

    lcd_init();
    lcd_clear();
    keypad_init();
    display_message("Mode: Admin");

    while (1) {
        key = keypad_getkey();
        if (mode == 'A') { // Admin mode
            if (key == '*') { // Add user
                add_user();
            } else if (key == '#') { // Update meds
                update_meds();
            } else if (key == 'D') { // Display meds
                display_meds();
            } else if (key >= '0' && key <= '9') {
                if (buf_index < sizeof(buffer) - 1) { 
										buffer[buf_index++] = key; 
										buffer[buf_index] = '\0'; 
										update_buffer_display(buffer); // Show the typed number on LCD
								}
						} else if (key == 'A') { 
								if (strcmp(buffer, "456") == 0) { 
										mode = 'U'; 
										display_message("Mode: User");
										buf_index = 0; 
										memset(buffer, 0, sizeof(buffer)); // Reset buffer after entering user mode
								} else { 
										display_message("Invalid Input"); 
										buf_index = 0; 
										memset(buffer, 0, sizeof(buffer)); // Reset buffer after incorrect input
								}
						}else {
                display_message("Mode: Admin");
            }
        } else if (mode == 'U') { // User mode
            if (key >= '0' && key <= '9') {
                if (buf_index < sizeof(buffer) - 1) {
                    buffer[buf_index++] = key;
                    buffer[buf_index] = '\0';
										update_buffer_display(buffer); // Show the typed number on LCD
                }
                if (strcmp(buffer, "123") == 0) {
                    mode = 'A';
                    display_message("Mode: Admin");
                    buf_index = 0;
                }
            } else if (key == 'A') { // Confirm ID
                int userID = atoi(buffer);
                buf_index = 0;
                display_message("Enter Meal:");
                while (1) {
                    key = keypad_getkey();
                    if (key >= '1' && key <= '3') {
                        dispense_meds(userID, key - '0');
                        break;
                    }
                }
            }
        }
    }
}
