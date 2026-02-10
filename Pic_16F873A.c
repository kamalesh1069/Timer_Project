#pragma config FOSC = HS
#pragma config WDTE = OFF
#pragma config PWRTE = ON
#pragma config BOREN = OFF
#pragma config LVP = OFF

#include <xc.h>
#define _XTAL_FREQ 20000000

// ================= GLOBAL VARIABLES =================
volatile unsigned char value = 0;
volatile unsigned char started = 0;
volatile unsigned char running = 0;

// ================= 7-SEGMENT (COMMON CATHODE) ======
const unsigned char seg_code[10] = {
    0x3F, 0x06, 0x5B, 0x4F, 0x66,
    0x6D, 0x7D, 0x07, 0x7F, 0x6F
};

// ================= INITIALIZATION ===================
void init_system(void)
{
    ADCON1 = 0x06;

    TRISD = 0x00;
    TRISC = 0x00;
    TRISB0 = 1;
    TRISB1 = 1;

    OPTION_REGbits.nRBPU = 0;

    // Ensure startup state is always 00 on display
    value = 0;
    started = 0;
    running = 0;

    // -------- TIMER1: EXACT 1 SECOND --------
    T1CONbits.TMR1CS = 0;  // Fosc/4
    T1CONbits.T1CKPS0 = 1; // 1:8 prescaler
    T1CONbits.T1CKPS1 = 1;
    T1CONbits.TMR1ON = 1;

    // 20MHz ? 1 second preload
    // Tick = 1.6us ? 625000 counts
    // 65536 - 62500 = 3036 (RELOAD 10 TIMES = 1s)
    TMR1 = 3036;

    TMR1IF = 0;
    TMR1IE = 1;
    PEIE = 1;
    GIE = 1;

    PORTD = 0;
    PORTC = 0;
}

// ================= DISPLAY ==========================
void update_display(unsigned char num)
{
    PORTC = seg_code[num / 10];
    PORTD = seg_code[num % 10];
}

// ================= SET BUTTON =======================
void check_set_button(void)
{
    if (RB0 == 0)
    {
        __delay_ms(30);
        if (RB0 == 0)
        {
            while (RB0 == 0)
                ;
            if (!started)
            {
                value = 20;
                started = 1;
            }
            else
                running = 1;
        }
    }
}

// ================= RESET BUTTON =====================
void check_reset_button(void)
{
    if (RB1 == 0)
    {
        __delay_ms(30);
        if (RB1 == 0)
        {
            while (RB1 == 0)
                ;
            if (value > 0)
                value--;
        }
    }
}

// ================= TIMER1 ISR ======================
void __interrupt() isr(void)
{
    static unsigned char tick10 = 0;

    if (TMR1IF)
    {
        TMR1IF = 0;
        TMR1 = 3036; // 0.1s tick
        tick10++;

        if (tick10 >= 10) // 10 × 0.1s = 1 second
        {
            tick10 = 0;

            if (running && value > 0)
            {
                value--;
                if (value == 0)
                    running = 0;
            }
        }
    }
}

// ================= MAIN =============================
void main(void)
{
    init_system();
    update_display(0);

    while (1)
    {
        update_display(value);
        check_set_button();
        check_reset_button();
    }
}
