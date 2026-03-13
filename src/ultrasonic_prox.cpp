#include <Arduino.h>

volatile bool update = false;
volatile uint16_t start = 0;
volatile uint16_t pulseTime = 0;

#define MAX_LENGTH 256
char send_buf[MAX_LENGTH];
volatile uint16_t send_len = 0;
volatile uint8_t err_status = 0;
volatile bool sending = false;
char temp1[20];
char temp2[20];

void setupTimer1PWM_100Hz_1ms() {

}
void setupTimer4InputCapture() {

}


void printBuffer() {
  cli();
  err_status = 0;
  sending = true;
  sei();
  UCSR0B |= (1 << UDRIE0);
}
void updateBuffer() {
  float ft = (pulseTime >> 1)*0.000698+0.0009;
  float in = modf(ft, NULL);
  dtostrf(in*12, 0, 4, temp1);
  send_len = snprintf(send_buf, MAX_LENGTH, "%dft %sin\n",(int)ft,temp1);
}
void setup() {
  //Pin modes
  DDRB = (1 << PB5);
  DDRL = 0;

  //Timer 1 FastPWM setup
  TCNT1 = 0;
  TCCR1A = (1 << WGM11) | (1 << COM1A1);
  TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS11);
  ICR1 = 19999;
  OCR1A = 2000;

  //Timer 4 input capture setup
  TCCR4A = 0;
  TCCR4B = (1 << ICES4) | (1 << CS41);
  TCCR4C = 0;
  TCNT4 = 0;
  TIFR4 = (1 << ICF4);
  TIMSK4 = (1 << ICIE4);

  //UART setup
  UCSR0A = 0b00000000;
  UCSR0B = 0b10011000;
  UCSR0C = 0b00000110;
  UBRR0H = 0b00000000;
  UBRR0L = 8;
}
void loop() {
  if (update) {
    cli();
    updateBuffer();
    update = false;
    pulseTime = 0;
    sei();
    printBuffer();
  }
}
ISR(TIMER4_CAPT_vect) {
  if (TCCR4B & (1 << ICES4)) {
    start = ICR4;
    TCCR4B &= ~(1 << ICES4);
    TIFR4 |= (1 << ICF4);
  } else {
    pulseTime = ICR4 - start;
    TCCR4B |= (1 << ICES4);
    TIFR4 |= (1 << ICF4);
    update = true;
  }
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