#include <Arduino.h>
//for CW, CLK lags behind DT
//for CCW, DT lags behind CLK
//if CW, each trigger of DT, DT & CLK will be equal
//by default the encoder is at 5V
//INT4 == D2 == PE4 -> CLK
//INT5 == D3 == PE5 -> DT

#define CLK_STATE (PINE & (1<<PE4)) >> PE4
#define DT_STATE (PINE & (1<<PE5)) >> PE5
#define MAX_LENGTH 128
char send_buf[MAX_LENGTH];
volatile uint8_t recv_len = 0;
volatile uint16_t send_len = 0;
volatile uint8_t err_status = 0;
volatile bool sending = false;
volatile bool new_packet = false;
volatile unsigned long CLK = 0;
volatile unsigned long DT = 0;
volatile long count = 0;
volatile bool update = false;
double revolutions = 0;
bool clockwise = false;
char buffer[128];
int PPR = 80;


void setup() {
    cli();
    DDRE = ~((1<<PE4)|(1<<PE5)); // assign inputs
    EICRB = 0b00000101; //trigger on any edge
    EIMSK = (1<<INT4)|(1<<INT5); //enable INT4 and INT5

    //setup USART
    UCSR0A = 0b00000000;
    UCSR0B = 0b10011000;
    UCSR0C = 0b00000110;
    UBRR0H = 0b00000000;
    UBRR0L = 8;
    sei();


}
void printBuffer() {
    cli();
    dtostrf(revolutions, 0, 4, send_buf);  // width=0, 4 decimals
    send_len = strlen(send_buf);
    send_buf[send_len] = '\n';
    send_len++;
    send_buf[send_len]   = '\0';
    recv_len = 0;
    err_status = 0;
    new_packet = false;
    sending = true;
    sei();
    UCSR0B |= (1 << UDRIE0);
}
void loop() {
    if (update) {
        revolutions = (double)count/PPR;
        printBuffer();
    }
    update = false;
}
ISR(INT4_vect) {
    if (CLK_STATE != DT_STATE) {
        clockwise = false;
        count--;
    } else {
        clockwise = true;
        count++;
    }
}
ISR(INT5_vect) {
    update = true;
    if (CLK_STATE != DT_STATE) {
        clockwise = true;
        count++;
    } else {
        clockwise = false;
        count--;
    }
}

ISR(USART0_UDRE_vect) {
    static uint16_t idx = 0;
    if (idx < send_len) {
        UDR0 = send_buf[idx++];
    } else {
        sending = false;
        idx = 0;
        UCSR0B &= ~(1 << 5);
    }
}