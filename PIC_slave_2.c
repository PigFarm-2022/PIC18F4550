#include "SoftWareUart.h"

sfr sbit SoftWare_Uart_Tx at RB2_bit;
sfr sbit SoftWare_Uart_Rx at RB3_bit;
sfr sbit SoftWare_Uart_Tx_Direction at TRISB2_bit;
sfr sbit SoftWare_Uart_Rx_Direction at TRISB3_bit;

float distance = 0;

float getDistance();
const int DISTANCE_CALLIBRATION = 1;

char distanceString[20];

int percentage;

char uart_rd;
char soft_uart_rd;

void timer1_delay(unsigned int time) {

  time /= 100; // divide the time x10.
  // so that 100ms x 10 = 1 sec.
  while (time) {
    TMR1H = 0x3C; // 100ms Timer Value.
    TMR1L = 0xB0;
    T1CON = 0x21;

    while (TMR1IF_bit == 0); // wait for timer-1 to overflow.
    TMR1IF_bit = 0; // clear timer-1 Flag.

    time--; // decrement timer value.
  }

}

float getDistance() {
  float distance;

  TMR1H = 0;
  TMR1L = 0;

  PORTA.F3 = 1;
  Delay_us(10);
  PORTA.F3 = 0;

  while (!PORTA.F4);
  T1CON.F0 = 1;
  while (PORTA.F4);
  T1CON.F0 = 0;

  distance = (TMR1L | (TMR1H << 8));
  distance = (TMR1L | (TMR1H << 8)) / 58.82;
  distance += DISTANCE_CALLIBRATION;
  if (distance >= 0 && distance <= 400) {
    return distance;
  }
  /*else {
                 return -1;
         }*/
}

long map(long x, long in_min, long in_max, long out_min, long out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void main() {

  int distance;

  ADCON1 = 0x0F; // Disable Analog Pins
  OSCCON = 0x77; // Use internal oscillator

  TRISD.F0 = 0;
  LATD.F0 = 1;

  TRISD.F1 = 0;
  LATD.F1 = 1;

  TRISD.F3 = 0;
  LATD.F3 = 1;

  TRISA.B3 = 0; /* Set RB0 pin as a digital output pin */
  PORTA.B3 = 0; /* Initially sets the RB0 pin as activ low */

  TRISA.B4 = 1; /* Set RB4 pin as a digital input pin */
  PORTA.B4 = 0; /* Initially sets the RB4 pin as activ low */

  //Power Indicator
  TRISD.F2 = 0;
  PORTD.F2 = 1;

  //UART Initialization

  UART1_Init(9600);
  Delay_ms(500);

  //SOftware UART 2 Initialization

  SoftSerial.Init();
  Delay_ms(100);

  while (1) {

    char uart_rd = UART1_Read();
    char soft_uart_rd = SoftSerial.GetChar();

    distance = getDistance();
    percentage = map(distance, 80, 7, 0, 100); // for FEED tank
    if (percentage <= 0) {
      percentage = 0;
    } else if (percentage >= 100) {
      percentage = 100;
    }
    IntToStr(percentage, distanceString);
    ltrim(distanceString);
    rtrim(distanceString);

    //CAGE 2 FEED 0.5KG

    if (uart_rd == 'A') {

      UART1_Write_Text("SSSSSSSS");
      UART1_Write('\n');
      UART1_Write('\r');

      LATD.F0 = 0;
      timer1_delay(4500);
      LATD.F0 = 1;

      UART1_Write('z');
      UART1_Write_Text(distanceString);
      UART1_Write('\n');
      UART1_Write('\r');

      uart_rd = 'o';
    }

    // Cage 2 Feed 1KG

    if (uart_rd == 'B') {

      UART1_Write_Text("SSSSSSSS");
      UART1_Write('\n');
      UART1_Write('\r');

      LATD.F0 = 0;
      timer1_delay(8500);
      LATD.F0 = 1;

      UART1_Write('z');
      UART1_Write_Text(distanceString);
      UART1_Write('\n');
      UART1_Write('\r');

      uart_rd = 'o';
    }

    // Cage 2 Feed 1.5KG

    if (uart_rd == 'C') {

      UART1_Write_Text("SSSSSSSS");
      UART1_Write('\n');
      UART1_Write('\r');

      LATD.F0 = 0;
      timer1_delay(12500);
      LATD.F0 = 1;

      UART1_Write('z');
      UART1_Write_Text(distanceString);
      UART1_Write('\n');
      UART1_Write('\r');

      uart_rd = 'o';

    }

    //CAGE 2 Wash

    if (uart_rd == 'W') {

      UART1_Write_Text("ssssssss");
      UART1_Write('\n');
      UART1_Write('\r');

      LATD.F1 = 0;
      timer1_delay(10000);
      LATD.F1 = 1;

      uart_rd = 'o';
    }

    //CAGE 2 Bath

    if (uart_rd == 'V') {

      UART1_Write_Text("ssssssss");
      UART1_Write('\n');
      UART1_Write('\r');

      LATD.F3 = 0;
      timer1_delay(30000);
      LATD.F3 = 1;

      uart_rd = 'o';
    }

    //CAGE 2 Thermal Sensor

    if (soft_uart_read == 'W') {

      UART1_Write_Text("ssssssss");
      UART1_Write('\n');
      UART1_Write('\r');

      LATD.F1 = 0;
      timer1_delay(30000);
      LATD.F1 = 1;

    }

    //CAGE 1 Refresh

    if (uart_rd == 'R') {

      timer1_delay(200);

      UART1_Write('z');
      UART1_Write_Text(distanceString);
      UART1_Write('\n');
      UART1_Write('\r');

      uart_rd = 'o';
    }
  }

}