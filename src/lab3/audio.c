
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
// The device address is based on the manufacturer and dev settings
#define AUDIO_DEV_ADDR 0xc0
#define MEAN 300

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
// i2c functionality
void i2c_init();
uint8_t i2c_write(uint8_t dev_addr, char *p, uint8_t n);
uint8_t i2c_wait();

// audio functionality
uint8_t audio_write(uint16_t dataVal);

// ADC interfacing
void adc_init();

uint16_t adc_read(uint8_t adcx);

uint16_t get_inc(uint16_t x, uint16_t y, uint16_t z);

// Sin lookup table
static const uint16_t table[] = 
     {0x200,0x20c,0x219,0x225,0x232,0x23e,0x24b,0x257,
      0x263,0x270,0x27c,0x288,0x294,0x2a0,0x2ac,0x2b8,
      0x2c3,0x2cf,0x2da,0x2e5,0x2f1,0x2fc,0x306,0x311,
      0x31c,0x326,0x330,0x33a,0x344,0x34e,0x357,0x360,
      0x369,0x372,0x37a,0x383,0x38b,0x393,0x39a,0x3a2,
      0x3a9,0x3b0,0x3b6,0x3bd,0x3c3,0x3c8,0x3ce,0x3d3,
      0x3d8,0x3dd,0x3e1,0x3e5,0x3e9,0x3ec,0x3f0,0x3f3,
      0x3f5,0x3f7,0x3f9,0x3fb,0x3fd,0x3fe,0x3fe,0x3ff,
      0x3ff,0x3ff,0x3fe,0x3fe,0x3fd,0x3fb,0x3f9,0x3f7,
      0x3f5,0x3f3,0x3f0,0x3ec,0x3e9,0x3e5,0x3e1,0x3dd,
      0x3d8,0x3d3,0x3ce,0x3c8,0x3c3,0x3bd,0x3b6,0x3b0,
      0x3a9,0x3a2,0x39a,0x393,0x38b,0x383,0x37a,0x372,
      0x369,0x360,0x357,0x34e,0x344,0x33a,0x330,0x326,
      0x31c,0x311,0x306,0x2fc,0x2f1,0x2e5,0x2da,0x2cf,
      0x2c3,0x2b8,0x2ac,0x2a0,0x294,0x288,0x27c,0x270,
      0x263,0x257,0x24b,0x23e,0x232,0x225,0x219,0x20c,
      0x200,0x1f3,0x1e6,0x1da,0x1cd,0x1c1,0x1b4,0x1a8,
      0x19c,0x18f,0x183,0x177,0x16b,0x15f,0x153,0x147,
      0x13c,0x130,0x125,0x11a,0x10e,0x103,0xf9,0xee,
      0xe3,0xd9,0xcf,0xc5,0xbb,0xb1,0xa8,0x9f,
      0x96,0x8d,0x85,0x7c,0x74,0x6c,0x65,0x5d,
      0x56,0x4f,0x49,0x42,0x3c,0x37,0x31,0x2c,
      0x27,0x22,0x1e,0x1a,0x16,0x13,0xf,0xc,
      0xa,0x8,0x6,0x4,0x2,0x1,0x1,0x0,
      0x0,0x0,0x1,0x1,0x2,0x4,0x6,0x8,
      0xa,0xc,0xf,0x13,0x16,0x1a,0x1e,0x22,
      0x27,0x2c,0x31,0x37,0x3c,0x42,0x49,0x4f,
      0x56,0x5d,0x65,0x6c,0x74,0x7c,0x85,0x8d,
      0x96,0x9f,0xa8,0xb1,0xbb,0xc5,0xcf,0xd9,
      0xe3,0xee,0xf9,0x103,0x10e,0x11a,0x125,0x130,
      0x13c,0x147,0x153,0x15f,0x16b,0x177,0x183,0x18f,
      0x19c,0x1a8,0x1b4,0x1c1,0x1cd,0x1da,0x1e6,0x1f3};

uint16_t sine(uint8_t x)
{
  return table[x];
}

int main(void)
{
   /* Setup serial port */
   uart_init();
   i2c_init();
   adc_init();
   audio_write(0);
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

   uint16_t time = 0;
   uint16_t x, y, z;
   while(1) {
      uint16_t value = sine(time);
      x = adc_read(0);
      y = adc_read(1);
      z = adc_read(2);
      if (time % 512 == 0)
        printf("x:%i y:%i z:%i\r\n", x, y, z);
      uint8_t error = audio_write(value);
      if (error)
      {
        printf("ERROR: Val:%x\n",error);
        while(1);
      }
      //time+= 31;
      time += get_inc(x, y, z);
   }
   while(1) {
      input = getchar();
      printf("You wrote %c\r\n", input);
      uint16_t val = sine((uint8_t) input);
      printf("sin(%i/%i) = %i\n", (int)input, 0xff, val);
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
  // Enable TWI
  TWCR = (1 << TWEN);
}

uint8_t i2c_write(uint8_t dev_addr, char *p, uint8_t n)
{
  uint8_t status;
  
  // Set the control register to set a start condition on the bus
  TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTA);
  // Wait for TWINT to get set
  status = i2c_wait();
  // Return if there was any status other than OK
  if (status != 0x08)
  {
    printf("step1 fail, %x\n", status);
    return(status);
  }


  // Now set the device address
  TWDR = dev_addr & 0xfe; // set r/w to 0
  TWCR = (1 << TWINT) | (1 << TWEN);
  status = i2c_wait();
  if (status != 0x18)
  {
    printf("step2 fail, %x\n", status);
    return(status);
  }


  // Now we need to actually write the data
  while (n-- > 0)
  {
    TWDR = *p++;
    TWCR = (1 << TWINT) | (1 << TWEN);
    status = i2c_wait();
    if (status != 0x28)
    {
      printf("step2 fail, %x\n", status);
      return(status);
    }
  }

  // Send the stop condition
  TWCR = (1 << TWINT) || (1 << TWEN) | (1 << TWSTO);
  return 0;
}

inline uint8_t i2c_wait()
{
  uint8_t status;
  while (!(TWCR & (1 << TWINT)));
  status = TWSR & 0xf8;
  return status;
}

uint8_t audio_write(uint16_t dataVal)
{
  char data[2];
  //zero out the higher order 4 bits to set (c2,c1) to (0,0)
  //and also set power mode to be (0,0), which is on
  data[0] = (char)0xf & (dataVal >> 8);
  data[1] = (char)0xff & dataVal;
  //Send the data to the audio device
  return i2c_write(AUDIO_DEV_ADDR, (char *)&data, 2);
}

//ADC functions
void adc_init()
{
  ADMUX |= (1 << REFS0);
  ADCSRA |= (1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0)|(1<<ADEN);
}

uint16_t adc_read(uint8_t adcx) {
  //adcx is the analog pin we want to read from
  ADMUX &= 0xf0;
  ADMUX |= adcx;

  ADCSRA |= _BV(ADSC);

  while (ADCSRA & _BV(ADSC));
  return ADC;
}

inline uint16_t absVal(uint16_t val)
{
  uint16_t av = MEAN - val > 1024 ? val - MEAN : MEAN - val;
  return av;
}

uint16_t get_inc(uint16_t x, uint16_t y, uint16_t z)
{
  x = absVal(x);
  y = absVal(y);
  z = absVal(z);
  if ((x >= y) && (x >= z))
  {
    return 30;
  }
  if ((y >= x) && (y >= z))
  {
    return 25;
  }
  if ((z >= x) && (z >= y))
  {
    return 41;
  }
  return 35;
}

