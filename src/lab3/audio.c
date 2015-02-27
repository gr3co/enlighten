
/* some includes */
#include <inttypes.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include <stdio.h>

#define BAUD 115200
#include <util/setbaud.h>

// Lab 3
// i2c
#define FOSC 9830400 // This is the oscillator frequency for this mpu
// We need to set the i2c controller's output frequency
// This is based off of the cpu frequency and the desired output
// frequency
// 100kHz is probably good
#define I2C_BAUD 100000
#define TWBR_VAL (FOSC  / I2C_BAUD - 16) / 2 + 1

void uart_init(void) {
   UBRR0H = UBRRH_VALUE;
   UBRR0L = UBRRL_VALUE;

#if USE_2X
   UCSR0A |= _BV(U2X0);
#else
   UCSR0A &= ~(_BV(U2X0));
#endif

   UCSR0C = _BV(UCSZ01) | _BV(UCSZ00); /* 8-bit data */
   UCSR0B = _BV(RXEN0) | _BV(TXEN0);   /* Enable RX and TX */
}

void uart_putchar(char c) {
   loop_until_bit_is_set(UCSR0A, UDRE0); /* Wait until data register empty. */
   UDR0 = c;
}

char uart_getchar(void) {
   loop_until_bit_is_set(UCSR0A, RXC0); /* Wait until data exists. */
   return UDR0;
}

FILE uart_output = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);
FILE uart_input = FDEV_SETUP_STREAM(NULL, uart_getchar, _FDEV_SETUP_READ);
FILE uart_io = FDEV_SETUP_STREAM(uart_putchar, uart_getchar, _FDEV_SETUP_RW);

// Lab 3 stuff
void i2c_init();

int main(void)
{

   /* Setup serial port */
   uart_init();
   stdout = &uart_output;
   stdin  = &uart_input;

   char input;

   // Setup ports
   DDRB |= (1<<1) | (1<<0);
   PORTB |= (1<<0);
   PORTB &= ~(1<<1);

   /* Print hello and then echo serial
   ** port data while blinking LED */
   printf("Hello world!\r\n");
   while(1) {
      input = getchar();
      printf("You wrote %c\r\n", input);
      PORTB ^= 0x01;
   }

}

void i2c_init()
{
  // Set the prescalar value to be 1
  // TWPS1 TWPS0 prescalar val
  //   0     0       1
  //   0     1       4
  //   1     0      16 
  //   1     1      64
  TWSR = 0;
  // Pull baud rate from preprocessor macro
  TWBR = TWBR_VAL; 
}


