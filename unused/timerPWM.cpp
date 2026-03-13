#include <Arduino.h>
void setup() {
    //pin setup
    DDRB = (1 << PB5);
    DDRF = 0;

    //ADC setup
    ADMUX  = (1 << REFS0);
    ADCSRA = (1 << ADEN) | (1 << ADATE) | (1 << ADSC) |
             (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
    ADCSRB = 0;
    ADCSRA |= (1 << ADSC);

    //timer and fast PWM setup
    TCCR1A = (1 << WGM11) | (1 << COM1A1);
    TCCR1B = (1 << WGM12) | (1 << WGM13);
    ICR1 = 3999;
    OCR1A = 2000;
    TCCR1B |= (1 << CS10);
}

void loop() {
    OCR1A = ICR1 * (float)ADC/1023.0 ;
}