    #include <Arduino.h>
    //for CW, CLK lags behind DT
    //for CCW, DT lags behind CLK
    //if CW, each trigger of DT, DT & CLK will be equal
    //by default the encoder is at 5V
    //INT4 == D2 == PE4 -> CLK
    //INT5 == D3 == PE5 -> DT
    #define SR 100.0
    #define CLK_STATE (PINE & (1<<PE4)) >> PE4
    #define DT_STATE (PINE & (1<<PE5)) >> PE5
    #define MAX_LENGTH 256
    #define ALPHA250 SR/(SR+250.0)
    #define ALPHA100 SR/(SR+100.0)
    #define ALPHA50 SR/(SR+50.0)
    #define ALPHA1 SR/(SR+1.0)
    char send_buf[MAX_LENGTH];
    volatile uint16_t send_len = 0;
    volatile uint8_t err_status = 0;
    volatile bool sending = false;
    volatile unsigned long CLK = 0;
    volatile unsigned long DT = 0;
    volatile long count = 0;
    volatile bool update = false;
    float angPos = 0;
    float angPosPrev = 0;
    float angVel250 = 0;
    float angVel100 = 0;
    float angVel50 = 0;
    float angVel1 = 0;
    float angVelPrev250 = 0;
    float angVelPrev100 = 0;
    float angVelPrev50 = 0;
    float angVelPrev1 = 0;
    bool clockwise = false;
    char buffer[128];
    char temp1[20];
    char temp2[20];
    char temp3[20];
    char temp4[20];
    char temp5[20];
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
        TCCR1A = 0;
        TCCR1B |= (1 << WGM12); // set to CTC
        // Prescaler = 64 -> 16MHz / 64 = 250 kHz
        TCCR1B |= (1 << CS11) | (1 << CS10);
        OCR1A = 2499; // 250kHz / 100Hz = 2500
        // Enable Timer1 Compare A Match interrupt
        TIMSK1 |= (1 << OCIE1A);
        sei();
    }
    void printBuffer() {
        cli();
        err_status = 0;
        sending = true;
        sei();
        UCSR0B |= (1 << UDRIE0);
    }
    void updateBuffer() {
        dtostrf(angPos, 0, 4, temp1);
        dtostrf(angVel250, 0, 4, temp2);
        dtostrf(angVel100, 0, 4, temp3);
        dtostrf(angVel50, 0, 4, temp4);
        dtostrf(angVel1, 0, 4, temp5);
        send_len = snprintf(send_buf, MAX_LENGTH, "%s, %s, %s, %s, %s\n", temp1, temp2, temp3, temp4, temp5);
    }
    void updateExtrinsics() {
        angPosPrev = angPos;
        angPos = (count/(float)PPR)*2*PI;
        angVelPrev250 = angVel250;
        angVel250 = angVelPrev250*(1-ALPHA250) + ALPHA250*(angPos - angPosPrev);
        angVelPrev100 = angVel100;
        angVel100 = angVelPrev100*(1-ALPHA100) + ALPHA100*(angPos - angPosPrev);
        angVelPrev50 = angVel50;
        angVel50 = angVelPrev50*(1-ALPHA50) + ALPHA50*(angPos - angPosPrev);
        angVelPrev1 = angVel1;
        angVel1 = angVelPrev1*(1-ALPHA1) + ALPHA1*(angPos - angPosPrev);
    }
    void loop() {
        if (update) {
            update = false;
            updateExtrinsics();
            updateBuffer();
            printBuffer();
        }
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
    ISR(TIMER1_COMPA_vect) {
        update = true;
    }