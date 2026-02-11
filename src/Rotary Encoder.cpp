#include <Arduino.h>
//for CW, CLK lags behind DT
//for CCW, DT lags behind CLK
//if CW, each trigger of DT, DT & CLK will be equal
//by default the encoder is at 5V
//INT4 == D2 == PE4 -> CLK
//INT5 == D3 == PE5 -> DT


volatile unsigned long CLK = 0;
volatile unsigned long DT = 0;
volatile long count = 0;
volatile bool update = false;
float revolutions = 0;
bool clockwise = false;
char buffer[128];
int PPR = 80;
#define CLK_STATE (PINE & (1<<PE4)) >> PE4
#define DT_STATE (PINE & (1<<PE5)) >> PE5

//todo: remove all calls to Serial and millis()
void setup() {
    cli();
    DDRE = ~((1<<PE4)|(1<<PE5)); // assign inputs
    EICRB = 0b00000101; //trigger on any edge
    EIMSK = (1<<INT4)|(1<<INT5); //enable INT4 and INT5
    sei();
    Serial.begin(115200);
}
void loop() {
    revolutions = (float)count/PPR;
    snprintf(buffer, 64, "%f",revolutions);
    Serial.println(revolutions);
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
    if (CLK_STATE != DT_STATE) {
        clockwise = true;
        count++;
    } else {
        clockwise = false;
        count--;
    }
}

