#include <Arduino.h>
#include <stdio.h>
#include <stdlib.h>

volatile uint8_t recv_buf[256];
volatile uint8_t send_buf[300];
volatile uint8_t recv_len = 0;
volatile uint16_t send_len = 0;
volatile uint8_t err_status = 0;
volatile bool new_data = false;
volatile bool sending = false;
unsigned int sqr_frequency = 2; // frequency in hz
void setup() {
    cli();
    // Setup USART
    UCSR0A = 0b00000000;
    UCSR0B = 0b10011000;
    UCSR0C = 0b00000110;
    UBRR0H = 0;
    UBRR0L = 103;
    DDRA = 0b11111111; //pins D22-29 for output
    ADMUX = 0b01000000;
    ADCSRA = 0b11100111;
    sei();
    Serial.begin(115200);
}

void loop() {
    unsigned long period = 1000000/sqr_frequency;
    //pulses pin 22 in accordance with the frequency
    if (micros()%period > period/2) {
        PORTA = 0b11111111;
    } else {
        PORTA = 0b00000000;
    }
    Serial.println(ADC);
}

ISR(USART0_RX_vect) {

}