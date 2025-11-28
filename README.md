# Associate Final Project - Electronic 6-Digit Electronic Lock(Atmega16L)
Electronic 6-digit lock -ATmega16L (CodeVision AVR). LCD, Keypad, EEPROM, rely, Buzzer.


# Electronic 6-digit Lock (ATmega16L)

Description
-----------
Embedded electronic lock using ATmega16L, 6-digit password stored in EEPROM.
Features: 4x3 keypad input, 16x2 LCD, buzzer, relay control, lockout after 3 failed attempts, change-password flow.

Hardware
--------
- MCU: ATmega16L @1MHz
- Keypad: 4x3 (Rows -> PD3..PD6, Cols -> PD0..PD2)
- LCD: 16x2 (data bus -> PORTB, control lines per lcd.h)
- Relay driver: PC1 (with transistor + flyback diode)
- Buzzer: PC0
- Alarm LED: PC2
- Error LED: PC3

How to build (CodeVision AVR)
-----------------------------
1. Create a new project in CodeVision AVR for ATmega16.
2. Add electronic_lock.c to the project.
3. Ensure includes mega16.h, delay.h, lcd.h are available.
4. Set CPU frequency to 1 MHz.
5. Build and program the device (generate HEX and flash).

Usage / Test cases
------------------
- Default password: 123456
- Press * to enter password mode (open)
- Press # to change password
- After 3 wrong attempts the system locks (reset required)

Files
-----
- electronic_lock.c  — main source
- Circuit Schematic.jpeg — schematic image
- README.md
- LICENSE

Notes
-----
- This code is written for CodeVision AVR. For avr-gcc, replace delay/EERPOM APIs accordingly.
- Check wiring before powering (especially relay/buzzer).
