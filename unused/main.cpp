#import <Arduino.h>
volatile uint8_t recv_buf[256];
volatile uint8_t send_buf[300];
volatile uint8_t recv_len = 0;
volatile uint16_t send_len = 0;
volatile uint8_t err_status = 0;
volatile bool new_data = false;
volatile bool sending = false;
void setup() {
    cli();
    // Setup USART
    UCSR0A = 0b00000000;
    UCSR0B = 0b10011000;
    UCSR0C = 0b00000110;
    UBRR0H = 0;
    UBRR0L = 103;
    sei();
}

void loop() {
    if (new_data) {
        while (sending); // wait for current transmission to complete
        cli();
        send_len = snprintf(send_buf, 300, "Received %d bytes (err code: %d): \"%s\"\n", recv_len, err_status, recv_buf);
        // Reset receive buffer
        recv_len = 0;
        err_status = 0;
        new_data = false;
        sei();
        // enable transmission
        sending = true;
        UCSR0B |= (1 << 5);
    }
}

ISR(USART0_RX_vect) {
    if (!new_data) {
        // Add incoming byte to buffer
        err_status = UCSR0A & 0b00011100;
        recv_buf[recv_len++] = UDR0;

        // Check for end of buffer
        if (recv_buf[recv_len - 1] == '\n') {
            recv_buf[recv_len - 1] = '\0';
            new_data = true;
        }
        else if (recv_len == 255)
        {
            recv_buf[recv_len] = '\0';
            new_data = true;
        }
    }
    else {
        // Ignore incoming data
        uint8_t dummy = UDR0;
    }
}

ISR(USART0_UDRE_vect) {
    static uint16_t idx = 0;
    if (idx < send_len) {
        UDR0 = send_buf[idx++]; // Send if there is data to be sent
    }
    else{
        // Disable transmission interrupts if no more data
        idx = 0;
        sending = false;
        UCSR0B &= ~(1 << 5);
    }
}
