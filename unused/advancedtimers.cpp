#include <Arduino.h>

#define BINS 256
#define SAMPLING_RATE 1000
#define MAX_LENGTH 256
char send_buf[MAX_LENGTH];
volatile uint16_t send_len = 0;
volatile uint8_t err_status = 0;
volatile bool sending = false;
char temp1[20];
char temp2[20];
volatile uint16_t i = 0;
uint8_t sineTable[BINS];
volatile bool update = false;
volatile int adcValue;
void setup() {
    //pre-compute sine table
    for (int n = 0; n < BINS; n++) {
        sineTable[n] = (uint8_t)(127.5 * sin(2.0 * PI * n / BINS) + 127.5);
    }
    //pin setup
    DDRA = 0b11111111;
    PORTA = 0;

    //timer setup
    cli();
    //sine generation timer
    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1 = 0;
    OCR1A = 624;
    TCCR1B |= (1 << WGM12);
    TCCR1B |= (1 << CS10);
    TIMSK1 |= (1 << OCIE1A);

    //sampling timer
    TCCR3A = 0;
    TCCR3B = 0;
    TCNT3  = 0;
    // 16 MHz / 1024 = 15625 Hz
    OCR3A = 15624 / SAMPLING_RATE;
    TCCR3B |= (1 << WGM32);  // CTC
    TCCR3B |= (1 << CS32) | (1 << CS30);
    TIMSK3 |= (1 << OCIE3A);

    //ADC setup
    ADMUX  = (1 << REFS0);
    ADCSRA = (1 << ADEN) | (1 << ADATE) | (1 << ADSC) |
             (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
    ADCSRB = 0;
    sei();
    ADCSRA |= (1 << ADSC);

    //UART setup
    UCSR0A = 0b00000000;
    UCSR0B = 0b10011000;
    UCSR0C = 0b00000110;
    UBRR0H = 0b00000000;
    UBRR0L = 8;
}
void printBuffer() {
    cli();
    err_status = 0;
    sending = true;
    sei();
    UCSR0B |= (1 << UDRIE0);
}
void updateBuffer() {
    send_len = snprintf(send_buf, MAX_LENGTH, "%d\n", adcValue);
    }
void loop() {
    if (update) {
        update = false;
        updateBuffer();
        printBuffer();
    }
}

ISR(TIMER1_COMPA_vect) {
    PORTA = sineTable[i];
    i++;
    if (i >= BINS) i = 0;
}

ISR(TIMER3_COMPA_vect) {
    update = true;
    adcValue = ADC;
}

ISR(USART0_UDRE_vect) {
    static uint16_t idx = 0;
    if (idx < send_len) {
        UDR0 = send_buf[idx];
        idx++;
    } else {
        sending = false;
        idx = 0;
        UCSR0B &= ~(1 << 5);
    }
}