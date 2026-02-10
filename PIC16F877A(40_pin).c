#include <xc.h>

#pragma config FOSC = HS
#pragma config WDTE = OFF
#pragma config PWRTE = ON
#pragma config BOREN = OFF
#pragma config LVP = OFF

#define _XTAL_FREQ 20000000

volatile unsigned char value = 20;
volatile unsigned char running = 0;
volatile unsigned char halfSec = 0;
volatile unsigned char sec_count = 0;

const unsigned char seg_code[10] =
{
    0x3F,0x06,0x5B,0x4F,0x66,
    0x6D,0x7D,0x07,0x7F,0x6F
};

void init_system(void)
{
    ADCON1 = 0x06;

    TRISA = 0x03;   // RA0 SET, RA1 RESET
    TRISB = 0x00;   // Tens
    TRISC = 0x00;   // Units

    PORTA = 0;
    PORTB = 0;
    PORTC = 0;

    // TIMER1 setup
    T1CON = 0x31;       // 1:8 prescaler

    TMR1H = 0x85;       // 0.5 second preload
    TMR1L = 0xEE;

    TMR1IF = 0;
    TMR1IE = 1;
    PEIE = 1;
    GIE = 1;
}

void update_display(unsigned char num)
{
    PORTB = seg_code[num / 10];
    PORTC = seg_code[num % 10];
}

void __interrupt() isr(void)
{
    if (TMR1IF)
    {
        TMR1IF = 0;

        T1CONbits.TMR1ON = 0;  // Stop timer

        TMR1H = 0x85;
        TMR1L = 0xEE;

        T1CONbits.TMR1ON = 1;  // Restart

        halfSec++;

        if(halfSec >= 2)   // 1 second
        {
            halfSec = 0;
            sec_count++;

            if(sec_count >= 60)
            {
                sec_count = 0;

                if(running && value > 0)
                {
                    value--;
                    if(value == 0)
                        running = 0;
                }
            }
        }
    }
}

void check_buttons(void)
{
    // RESET (manual decrement)
    if(RA1 == 0)
    {
        __delay_ms(30);
        if(RA1 == 0)
        {
            while(RA1 == 0);
            if(value > 0)
                value--;
        }
    }

    // SET (start)
    if(RA0 == 0)
    {
        __delay_ms(30);
        if(RA0 == 0)
        {
            while(RA0 == 0);
            running = 1;
        }
    }
}

void main(void)
{
    init_system();

    while(1)
    {
        update_display(value);
        check_buttons();
    }
}
