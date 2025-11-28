/************************
  Chip Type: ATmega16L
  Program type: Application
  Clock frequency: 1 MHz
  CodeVision AVR style
***********************/

#include <mega16.h>
#include <delay.h>
#include <lcd.h>
#include <string.h>

/* -------------------------------
   EEPROM-stored password
   - In CodeVision AVR, declaring 'char eeprom X[]' places it in EEPROM.
   - We initialize with a default 6-digit code (this value stored in EEPROM at compile time).
   --------------------------------*/
#pragma warn -par
char eeprom saved_password[6] = { '1','2','3','4','5','6' };  /* default password */
#pragma warn +par

/* Runtime RAM copies */
char password_entry[6];
char password_temp[6];
unsigned char key_val;            /* last key value from keypad */
unsigned char attempts;           /* number of failed attempts */
bit access_granted;               /* 0/1 flag */

/* Function prototypes */
unsigned char keypad(void);
void delay_debounce(void);
unsigned char password_check(void);
void new_password(void);
void load_password_from_eeprom(void);
void store_password_to_eeprom(void);

/* Helper: map numeric key id to ASCII character for display/storage */
char key_to_char(unsigned char key)
{
    switch(key) {
        case 1: return '1';
        case 2: return '2';
        case 3: return '3';
        case 4: return '4';
        case 5: return '5';
        case 6: return '6';
        case 7: return '7';
        case 8: return '8';
        case 9: return '9';
        case 10: return '';   / row4 col0 */
        case 0: return '0';    /* row4 col1 */
        case 11: return '#';   /* row4 col2 */
        default: return 0;
    }
}

/* ------------------------------
   Main
   ------------------------------*/
void main(void)
{
    /* PORT configuration based on your schematic:
       - PORTB used for LCD data (DB0..DB7)
       - PORTC used as outputs for LEDs, buzzer, relay (PC0..PC3)
       - PORTD:
         PD0..PD2 = keypad columns (inputs with pull-ups)
         PD3..PD6 = keypad rows (outputs driven low/high)
         PD7 unused or general input (we keep as input)
    */

    DDRB = 0x00;   /* LCD data as input to start; lcd_init will handle */
    PORTB = 0x00;

    DDRC = 0xFF;   /* PC0..PC7 as outputs for LEDs/buzzer/relay */
    PORTC = 0x00;   /* all off initially */

    DDRD = 0x78;   /* 0b01111000 : PD3..PD6 as outputs (rows), PD0..PD2 inputs (cols) */
    /* Enable pull-ups on PD0..PD2 only */
    PORTD = 0x07;  /* 0b00000111 : pull-ups enabled on PD0..PD2, rows idle high */

    /* Init LCD */
    lcd_init(16);
    lcd_clear();

    /* load password from EEPROM (copy to RAM area if desired) */
    load_password_from_eeprom();

    attempts = 0;
    access_granted = 0;

    /* Main menu loop */
    while (1)
    {
        /* Show simple menu:
           *  press '*' to enter (open)  -> star mapped to key 10 in mapping
           *  press '#' to change password -> hash mapped to key 11
        */
        lcd_clear();
        lcd_putsf("Enter: *  Change: #");
        lcd_gotoxy(0,1);
        lcd_putsf("Press key...");

        /* wait for a key */
        unsigned char k = keypad();
        if (k == 10) { /* '*' -> enter password mode */
            access_granted = password_check();
            if (access_granted) {
                /* Activate relay and show accepted */
                lcd_clear();
                lcd_putsf("Pass accepted");
                PORTC.1 = 1;    /* relay ON (example: PC1) */
                delay_ms(1500);
                PORTC.1 = 0;    /* relay OFF */
                /* reset attempts */
                attempts = 0;
            } else {
                /* failure handling done inside password_check */
            }
        } else if (k == 11) { /* '#' -> change password flow */
            new_password();
        } else {
            /* ignore others */
        }

        delay_ms(300);
    }
}

/* ------------------------------
   Read EEPROM-stored password into RAM if needed
   (for now we keep using saved_password EEPROM directly for comparisons)
   ------------------------------*/
void load_password_from_eeprom(void)
{
    /* In CodeVision the eeprom variable can be read directly, but for
       clarity we copy into a RAM buffer if needed.
       Here we do nothing because we compare directly with saved_password.
    */
}

/* Write saved_password[] in EEPROM already occurs by assigning saved_password[]
   if you want to explicitly write you would use CodeVision EEPROM API.
   We'll provide store function for clarity in case multiple writes needed.
*/
void store_password_to_eeprom(void)
{
    /* In CodeVision, assigning to eeprom array elements writes them to EEPROM.
       So we simply write saved_password[i] = password_temp[i] etc.
       Here this function does nothing extra because calling code assigns to saved_password.
       If using avr-gcc, replace with eeprom_write_byte(&saved_password[i], byte).
    */
}

/* ------------------------------
   password_check(): reads 6 digits from keypad,
   compares with saved_password (EEPROM), limits attempts to 3,
   sets attempts counter and returns 1 if accepted, 0 otherwise.
   ------------------------------*/
unsigned char password_check(void)
{
    unsigned char i;
    unsigned char match_count;
    unsigned char k;

    lcd_clear();
    lcd_putsf("Enter password:");
    lcd_gotoxy(0,1);

    /* read 6 digits */
    for (i = 0; i < 6; i++) {
        k = keypad();                      /* wait and return key code */
        password_entry[i] = key_to_char(k);
        lcd_putchar('*');
    }
    delay_ms(300);

    /* compare with EEPROM saved_password */
    match_count = 0;
    for (i = 0; i < 6; i++) {
        if (password_entry[i] == saved_password[i]) match_count++;
    }

    if (match_count == 6) {
        /* accepted */
        lcd_clear();
        lcd_putsf("Pass accepted...");
        PORTC.1 = 1;    /* relay */
        PORTC.0 = 1;    /* buzzer on briefly */
        delay_ms(300);
        PORTC.0 = 0;
        delay_ms(1200);
        PORTC.1 = 0;
        attempts = 0;
        return 1;
    } else {
        /* not accepted */
        attempts++;
        lcd_clear();
        lcd_putsf("Invalid Pass!");
        PORTC.3 = 1; /* Red LED on */
        PORTC.0 = 1; /* buzzer short */
        delay_ms(500);
        PORTC.0 = 0;
        PORTC.3 = 0;

        if (attempts >= 3) {
            /* lockout and alarm */
            lcd_clear();
            lcd_putsf("Too many tries!");
            PORTC.2 = 1; /* persistent alarm LED or similar */
            /* sound buzzer for longer */
            PORTC.0 = 1;
            delay_ms(3000);
            PORTC.0 = 0;
            /* system locked until reset - simulate by waiting indefinitely */
            while (1) {
                /* could implement external reset input reading here */
                delay_ms(1000);
            }
        }
        return 0;
    }
}

/* ------------------------------
   new_password(): user enters old password, then new password twice to confirm
   ------------------------------*/
void new_password(void)
{
    unsigned char i, k;
    unsigned char ok_old = 0;
    unsigned char confirm_ok = 0;

    lcd_clear();
    lcd_putsf("Change password");
    delay_ms(500);

    /* ask for old password */
    lcd_clear();
    lcd_putsf("Enter old pass:");
    lcd_gotoxy(0,1);
    for (i = 0; i < 6; i++) {
        k = keypad();
        password_entry[i] = key_to_char(k);
        lcd_putchar('*');
    }
    delay_ms(300);

    /* compare */
    ok_old = 1;
    for (i = 0; i < 6; i++) {
        if (password_entry[i] != saved_password[i]) { ok_old = 0; break; }
    }

    if (!ok_old) {
        lcd_clear();
        lcd_putsf("Old pass wrong!");
        PORTC.0 = 1; delay_ms(300); PORTC.0 = 0;
        return;
    }

    /* get new password */
    lcd_clear();
    lcd_putsf("Enter new pass:");
    lcd_gotoxy(0,1);
    for (i = 0; i < 6; i++) {
        k = keypad(); password_temp[i] = key_to_char(k); lcd_putchar('*');
    }
    delay_ms(300);

    /* confirm */
    lcd_clear();
    lcd_putsf("Confirm pass:");
    lcd_gotoxy(0,1);
    for (i = 0; i < 6; i++) {
        k = keypad(); password_entry[i] = key_to_char(k); lcd_putchar('*');
    }
    delay_ms(300);

    confirm_ok = 1;
    for (i = 0; i < 6; i++) {
        if (password_entry[i] != password_temp[i]) { confirm_ok = 0; break; }
    }

    if (confirm_ok) {
        /* write to EEPROM */
        for (i = 0; i < 6; i++) {
            saved_password[i] = password_temp[i]; /* in CodeVision this writes to EEPROM */
        }
        store_password_to_eeprom(); /* placeholder if extra actions needed */
        lcd_clear();
        lcd_putsf("Pass changed..");
        delay_ms(1200);
        lcd_clear();
    } else {
        lcd_clear();
        lcd_putsf("Confirm mismatch!");
        PORTC.0 = 1; delay_ms(400); PORTC.0 = 0;
        delay_ms(400);
    }
}

/* ------------------------------
   keypad(): scans 4x3 keypad connected as:
     Rows -> PD3, PD4, PD5, PD6  (outputs)
     Cols -> PD0, PD1, PD2       (inputs with pull-ups)
   returns numeric code:
     1..9 numbers, 10='*', 0='0', 11='#'
   Includes debounce
   ------------------------------*/
unsigned char keypad(void)
{
    unsigned char key = 255; /* invalid */
    unsigned char row;
    unsigned char col;
    unsigned int timeout;

    /* Ensure rows are high (inactive) initially */
    PORTD |= (1<<3)|(1<<4)|(1<<5)|(1<<6);

    while (1) {
        /* scan each row by pulling it low and checking columns */
        for (row = 0; row < 4; row++) {
            /* set all rows high */
            PORTD |= (1<<3)|(1<<4)|(1<<5)|(1<<6);
            /* pull current row low */
            PORTD &= ~((1<<(3+row)));

            delay_ms(5); /* tiny settle */

            /* read columns */
            if ((PIND & 0x01) == 0) { col = 0; }      /* PD0 low */
            else if ((PIND & 0x02) == 0) { col = 1; } /* PD1 low */
            else if ((PIND & 0x04) == 0) { col = 2; } /* PD2 low */
            else { col = 0xFF; }

            if (col != 0xFF) {
                /* Debounce: wait to confirm stable press */
                delay_ms(30);
                /* re-check */
                if ((row==0 && ((PIND & 0x01)==0 || (PIND & 0x02)==0 || (PIND & 0x04)==0)) ||
                    (row==1 && ((PIND & 0x01)==0 || (PIND & 0x02)==0 || (PIND & 0x04)==0)) ||
                    (row==2 && ((PIND & 0x01)==0 || (PIND & 0x02)==0 || (PIND & 0x04)==0)) ||
                    (row==3 && ((PIND & 0x01)==0 || (PIND & 0x02)==0 || (PIND & 0x04)==0)) )
                {
                    /* map row,col to key */
                    if (row==0 && col==0) key = 1;
                    if (row==0 && col==1) key = 2;
                    if (row==0 && col==2) key = 3;
                    if (row==1 && col==0) key = 4;
                    if (row==1 && col==1) key = 5;
                    if (row==1 && col==2) key = 6;
                    if (row==2 && col==0) key = 7;
                    if (row==2 && col==1) key = 8;
                    if (row==2 && col==2) key = 9;
                    if (row==3 && col==0) key = 10;  /* '*' */
                    if (row==3 && col==1) key = 0;   /* '0' */
                    if (row==3 && col==2) key = 11;  /* '#' */

                    /* wait for key release before returning */
                    while (((PIND & 0x07) != 0x07)) { /* any column low */
                        delay_ms(10);
                    }
                    delay_ms(30);
                    return key;
                }
            }
        } /* end for rows */
    } /* end while */
}

/* ------------------------------
   small debounce helper (not used in current code but kept)
   ------------------------------*/
void delay_debounce(void)
{
    delay_ms(30);
}

/* End of file */
