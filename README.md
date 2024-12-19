# Project Description

The Automatic Medicine Dispensor System is a microcontroller-based project designed to manage medication storage and dispensing for multiple users. It uses an LPC23xx microcontroller, a 4x4 keypad for input, and an LCD for displaying messages. The system allows an administrator to add users, update medication stock, and monitor inventory. Users can request their prescribed medications based on their ID and meal type.

Features

# Admin Mode

Add User: The admin can add new users by providing their user ID, medication requirements for each compartment, and meal type.
Update Medication Stock: Admins can replenish medication stock in specific compartments.
Display Medication Stock: View the current stock levels for all compartments.

Switch Modes:Switch to User Mode by entering the admin password (456).

# User Mode

Request Medication: Users can enter their user ID and select a meal type to receive their prescribed medications.
Switch Modes: Switch back to Admin Mode by entering the user password (123).

# Components

Microcontroller: LPC23xx
Keypad: 4x4 matrix keypad
LCD Display: For displaying messages and prompts
Medication Compartments: Simulated in the code using arrays
Power Supply: Suitable power source for the microcontroller and peripherals

# Code Explanation

Initialization

keypad_init(): Configures the keypad pins.
lcd_init() and lcd_clear(): Initialize and clear the LCD for display.

Admin Functions

add_user(): Adds a new user by taking input for user ID, medication requirements, and meal type.
update_meds(): Updates the stock for selected compartments.
display_meds(): Displays the current stock levels for all compartments on the LCD.

User Functions

dispense_meds(int userID, int meal): Dispenses medications based on the user ID and meal type. Validates stock levels before dispensing.

Utility Functions

keypad_getkey(): Reads and returns the key pressed on the keypad.
display_message(const char *message): Displays a static message on the LCD.
update_buffer_display(char *buffer): Dynamically updates the LCD with user input.

Modes

Admin Mode (mode = 'A'): Provides access to all administrative functionalities.
User Mode (mode = 'U'): Allows users to request medications.

# How It Works

Startup:

The system initializes in Admin Mode.
Displays "Mode: Admin" on the LCD.
Admin Mode Operations:

* Key: Add a new user.
 Key: Update medication stock.
D Key: Display current stock levels.
Entering 456: Switches to User Mode.

User Mode Operations:

Enter user ID and select a meal type (1: Breakfast, 2: Lunch, 3: Dinner).
The system checks stock levels and dispenses medications if available.
Entering 123: Switches back to Admin Mode.

Error Handling:

Displays error messages for invalid user IDs, meal selections, or insufficient stock.
Usage

Add Users:

Admin presses * and enters user details.
Replenish Stock:
Admin presses # and selects compartments to update stock.
Dispense Medication:

Users input their ID and meal type in User Mode.

Switch Modes:

Admin to User: Enter password 456.

User to Admin: Enter password 123.
