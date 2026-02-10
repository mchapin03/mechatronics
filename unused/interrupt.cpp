#include <Arduino.h>
unsigned long count = 0;
void setup() {
    DDRD = 0;
    PORTD = (1<<2);
    cli();
    EICRA = 0b00100000;
    EIMSK = 0b00000100;
    sei();
    Serial.begin(9600);
}
void loop() {
    Serial.println(count);
}

ISR(INT2_vect) {
    count++;
}