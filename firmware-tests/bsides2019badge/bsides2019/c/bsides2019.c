#include <avr/io.h>
#include <util/delay.h>

#define BAUD 19200
#define BAUD_PRESCALLER (((F_CPU / (BAUD * 16UL))) - 1)

// init the UART using magic BAUD_PRESCALER macro
void USART_init(void) {
    UBRR0H = (uint8_t)(BAUD_PRESCALLER>>8);
    UBRR0L = (uint8_t)(BAUD_PRESCALLER);
    UCSR0B = (1<<RXEN0)|(1<<TXEN0);
    UCSR0C = ((1<<UCSZ00)|(1<<UCSZ01));
}

// send UART byte
void USART_send(unsigned char tx) {
    while(!(UCSR0A & (1<<UDRE0)));
         UDR0 = tx;
}

// send UART string
void USART_print(char* txdata) {
    while(*txdata != 0x00) {
	USART_send(*txdata);
	txdata++;
    }
}

int main(void) {

    //init UART
    USART_init();
    PORTB = 0xFF;
    PORTC = 0xFF;
    PORTD = 0xFF;

    for(;;){
	USART_print("Testing\n\r");
	PORTB ^= 0xFF;
	PORTC ^= 0xFF;
	PORTD ^= 0xFF;
	_delay_ms(1000);
    }

    return 0;
}

