#import <Arduino.h>
void setup() {
    cli();
    TCCR1A = 0;
    TCCR1B = 0b00011100;
    TCCR1C = 0;
    ICR1 = 31249;
    TIMSK1 = 0b00000010;
    OCR1A = 0;
    sei();
    Serial.begin(9600);
}
void loop() {};
ISR(TIMER1_COMPA_vect) {
    Serial.println("e");

}
