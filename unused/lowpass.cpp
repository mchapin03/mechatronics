#include <Arduino.h>
#include <stdio.h>

#define frequency_hz 1000
#define rxBufferFull (UCSR0A & (1<<RXC0))
#define txBufferEmpty (UCSR0A & (1<<UDRE0))
#define maxLength 128
#define errorStatus (UCSR0A & 0b00001110)

char rxBuffer[maxLength];
char txBuffer[maxLength];
unsigned int rxLength = 0;
unsigned int txLength = 0;

void setup() {
    DDRA = 0b11111111; //pins D22-29 for output

    ADMUX  = 0b01000000; //sets voltage reference and pin A0
    ADCSRA = 0b11100111;

    UCSR0A = 0b00000000;
    UCSR0B = 0b00011000; //enables the USART receiver (bit 4) and transmitter (bit 3)
    //sets USART to async (bits 7 and 6 as 0), even parity (bit 5 as 0, bit 4 as 0)
    //8 bit character size, 1 stop bit (bit 3 as 0)
    UCSR0C = 0b00110110;
    UBRR0H = 0;
    UBRR0L = 103; // set BAUD to 9600

}

void print(char* string) {
    txLength = snprintf(txBuffer, maxLength, "%s", string);
    for (unsigned int i = 0; i < txLength; i++) {
        while (!txBufferEmpty){}
        UDR0 = txBuffer[i];
    }
}

char* read() {
    rxLength = 0;
    for (unsigned int i = 0; i <)
        while (!rxBufferFull) {}
        if (UDR0 == )

}

void loop() {
    char* str = read();
    print(str);
}
