PIC16F873A(28 PIN) FULLY FUNCTIONAL CODE


#include <xc.h>

#pragma config FOSC = HS
#pragma config WDTE = OFF
#pragma config PWRTE = ON
#pragma config BOREN = OFF
#pragma config LVP = OFF

/*
 * Real-board calibration note:
 * If your real stopwatch shows one firmware "minute" = ~75 s,
 * then each firmware "second" is effectively ~1.25 s on your board setup.
 * To force real-time 60 s decrement without changing hardware right now,
 * we calibrate the minute threshold from 60 to 48 (60 / 1.25 = 48).
 *
 * When oscillator/hardware clock is fully corrected, set this back to 60.
 */
#define REAL_SECONDS_PER_DECREMENT 48U

#define _XTAL_FREQ 16000000UL

volatile unsigned char number = 0;
volatile unsigned char running = 0;
volatile unsigned char tick = 0;
volatile unsigned char secondCount = 0;

const unsigned char seg[10] =
{
    0x3F,0x06,0x5B,0x4F,0x66,
    0x6D,0x7D,0x07,0x7F,0x6F
};

void initSystem(void)
{
    ADCON1 = 0x06;

    TRISA = 0x03;   // RA0, RA1 input
    TRISB = 0x00;   // Tens
    TRISC = 0x00;   // Units

    PORTA = 0;
    PORTB = 0;
    PORTC = 0;

    // ---- TIMER1 SETUP ----
    // Fosc = 16 MHz => instruction clock = Fosc/4 = 4 MHz
    // Timer1 tick with 1:8 prescaler = 2 us
    // For 100 ms interval => 100000 / 2 = 50000 counts
    // preload = 65536 - 50000 = 15536 = 0x3CB0
    T1CON = 0x31;     // Timer1 ON, internal clock, 1:8 prescaler
    TMR1H = 0x3C;
    TMR1L = 0xB0;

    TMR1IF = 0;
    TMR1IE = 1;
    PEIE = 1;
    GIE = 1;
}

void updateDisplay(unsigned char value)
{
    PORTB = seg[value / 10];
    PORTC = seg[value % 10];
}

void __interrupt() ISR(void)
{
    if (TMR1IF)
    {
        TMR1IF = 0;

        // Keep timer running; just reload as quickly as possible.
        TMR1H = 0x3C;
        TMR1L = 0xB0;

        if (running)
        {
            tick++;

            if (tick >= 10)          // 1 firmware second
            {
                tick = 0;
                secondCount++;

                if (secondCount >= REAL_SECONDS_PER_DECREMENT)
                {
                    secondCount = 0;

                    if (number > 0)
                    {
                        number--;

                        if (number == 0)
                        {
                            running = 0;
                        }
                    }
                    else
                    {
                        running = 0;
                    }
                }
            }
        }
    }
}

void checkButtons(void)
{
    // RESET (manual decrement)
    if (RA1 == 0)
    {
        __delay_ms(30);
        if (RA1 == 0)
        {
            while (RA1 == 0);

            if (number > 0)
            {
                number--;
            }
        }
    }

    // SET (load 20 and start countdown)
    if (RA0 == 0)
    {
        __delay_ms(30);
        if (RA0 == 0)
        {
            while (RA0 == 0);

            // Load 20 only when SET is pressed, then start timing from zero.
            number = 20;
            tick = 0;
            secondCount = 0;
            running = 1;
        }
    }
}

void main(void)
{
    initSystem();

    while (1)
    {
        updateDisplay(number);
        checkButtons();
    }
}
