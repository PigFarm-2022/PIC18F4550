#include "SoftWareUart.h"

sfr sbit SoftWare_Uart_Tx at RB2_bit;
sfr sbit SoftWare_Uart_Rx at RB3_bit;
sfr sbit SoftWare_Uart_Tx_Direction at TRISB2_bit;
sfr sbit SoftWare_Uart_Rx_Direction at TRISB3_bit;

char serialRecieved[20]; //character

char ultrasonicY[7];

int * result;
int time_taken;
char txt[7];
int txtInt;

sbit led1 at RD0_bit;

char data_, data_2;

char error, byte_read;

int i, j, k, l, m;

float distance = 0;

char txt_slave_recieve;
char txt_slave_send[7];

char outputY[20];
char outputZ[20];
char outputString[7];

float getDistance();
const int DISTANCE_CALLIBRATION = 1;

char distanceString[20];

int percentage;

char x[10];

unsigned long millis_count = 0;
unsigned long time1, time2;
unsigned long delay_sensor = 0;

char uart_rd;

void timer1_delay(unsigned int time) {

  time /= 100; // divide the time x10.
  // so that 100ms / 100 = 1 sec.
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

unsigned short read_ds1307(unsigned short address) {
  unsigned short r_data;
  I2C1_Start();
  I2C1_Wr(0xD0); //address 0x68 followed by direction bit (0 for write, 1 for read) 0x68 followed by 0 --> 0xD0
  I2C1_Wr(address);
  I2C1_Repeated_Start();
  I2C1_Wr(0xD1); //0x68 followed by 1 --> 0xD1
  r_data = I2C1_Rd(0);
  I2C1_Stop();
  return (r_data);
}

void write_ds1307(unsigned short address, unsigned short w_data) {
  I2C1_Start(); // issue I2C start signal
  //address 0x68 followed by direction bit (0 for write, 1 for read) 0x68 followed by 0 --> 0xD0
  I2C1_Wr(0xD0); // send byte via I2C (device address + W)
  I2C1_Wr(address); // send byte (address of DS1307 location)
  I2C1_Wr(w_data); // send data (data to be written)
  I2C1_Stop(); // issue I2C stop signal
}

unsigned char BCD2UpperCh(unsigned char bcd) {
  return ((bcd >> 4) + '0');
}

unsigned char BCD2LowerCh(unsigned char bcd) {
  return ((bcd & 0x0F) + '0');
}

int Binary2BCD(int a) {
  int t1, t2;
  t1 = a % 10;
  t1 = t1 & 0x0F;
  a = a / 10;
  t2 = a % 10;
  t2 = 0x0F & t2;
  t2 = t2 << 4;
  t2 = 0xF0 & t2;
  t1 = t1 | t2;
  return t1;
}

int BCD2Binary(int a) {
  int r, t;
  t = a & 0x0F;
  r = t;
  a = 0xF0 & a;
  t = a >> 4;
  t = 0x0F & t;
  r = t * 10 + r;
  return r;
}

int second;
int minute;
int hour;
int hr;
int day;
int dday;
int month;
int year;
int ap;

unsigned short set_count = 0;
short set;

char time[] = "00:00 PM 00";
char date[] = "00-00-00";

void main() {
  int distance;
  I2C1_Init(100000); //DS1307 I2C is running at 100KHz
  ADCON1 = 0x0F; // Disable Analog Pins
  //OSCCON = 0x77;        // Use internal oscillator

  TRISD.F1 = 0;
  PORTD.F1 = 1;

  //Power Indicator
  TRISD.F2 = 0;
  PORTD.F2 = 1;

  TRISA = 0b00010111;
  PORTA = 0;

  //UART Initialization

  UART1_Init(9600);
  Delay_ms(100);

  UART1_Write('o');

  //UART1_Write_Text("UART OK!");

  //SOftware UART 2 Initialization

  SoftSerial.Init();
  Delay_ms(100);

  SoftSerial.PutChar('d');
  for (i = 0; i < 11; i++) {
    SoftSerial.PutChar(date[i]);
  }
  SoftSerial.PutChar('\n');
  SoftSerial.PutChar('\r');
  Delay_ms(500);

  // Software UART Initialization

  /*error = Soft_UART_Init(&PORTB, 3, 2, 9600, 0);        // Initialize Soft UART at 9600 bps
  if (error > 0) {
     PORTB = error;                               // Signalize Init error
     while(1) ;                                   // Stop program
  }

  Delay_ms(100); */

  //T1CON = 0x10; /*Set timer1 prescaler value to 1:2 prscale and disable timer1 */

  while (1) {

    uart_rd = UART1_Read();

    if (uart_rd == 'y') {
      SoftSerial.PrintLine("AT\r\n");
      SoftSerial.PutChar(0x0D);
      timer1_delay(200);
      SoftSerial.PrintLine("ATE0\r\n");
      SoftSerial.PutChar(0x0D);
      timer1_delay(200);
      SoftSerial.PrintLine("AT+CMGF=1\r\n");
      SoftSerial.PutChar(0x0D);
      timer1_delay(200);
      SoftSerial.PrintLine("AT+CMGS=\"+639201887451\"\r\n"); // Mobile Number
      SoftSerial.PutChar(0x0D);
      timer1_delay(200);
      SoftSerial.PrintLine("Feed Tank 1 is 10% please refill!"); // Message
      SoftSerial.PutChar(0x1A);
      SoftSerial.PutChar(0x0D);
    }

    if (uart_rd == 'z') {
      SoftSerial.PrintLine("AT\r\n");
      SoftSerial.PutChar(0x0D);
      timer1_delay(200);
      SoftSerial.PrintLine("ATE0\r\n");
      SoftSerial.PutChar(0x0D);
      timer1_delay(200);
      SoftSerial.PrintLine("AT+CMGF=1\r\n");
      SoftSerial.PutChar(0x0D);
      timer1_delay(200);
      SoftSerial.PrintLine("AT+CMGS=\"+639201887451\"\r\n"); // Mobile Number
      SoftSerial.PutChar(0x0D);
      timer1_delay(200);
      SoftSerial.PrintLine("Feed Tank 2 is 10% please refill!"); // Message
      SoftSerial.PutChar(0x1A);
      SoftSerial.PutChar(0x0D);

    }
    //Set Time

    set = 0;
    if (PORTA.F0 == 0) {
      Delay_ms(50);
      if (PORTA.F0 == 0) {
        set_count++;
        if (set_count >= 7) {
          set_count = 0;
        }
      }
    }

    if (set_count) {
      if (PORTA.F1 == 0) {
        Delay_ms(50);
        if (PORTA.F1 == 0)
          set = 1;
      }

      if (PORTA.F2 == 0) {
        Delay_ms(50);
        if (PORTA.F2 == 0)
          set = -1;
      }

      if (set_count && set) {
        switch (set_count) {
        case 1:
          hour = BCD2Binary(hour);
          hour = hour + set;
          hour = Binary2BCD(hour);
          if ((hour & 0x1F) >= 0x13) {
            hour = hour & 0b11100001;
            hour = hour ^ 0x20;
          } else if ((hour & 0x1F) <= 0x00) {
            hour = hour | 0b00010010;
            hour = hour ^ 0x20;
          }

          write_ds1307(2, hour); //write hour
          break;
        case 2:
          minute = BCD2Binary(minute);
          minute = minute + set;
          if (minute >= 60)
            minute = 0;
          if (minute < 0)
            minute = 59;
          minute = Binary2BCD(minute);
          write_ds1307(1, minute); //write min
          break;

        case 3:
          if (abs(set))
            write_ds1307(0, 0x00); //Reset second to 0 sec. and start Oscillator
          break;
        case 4:
          day = BCD2Binary(day);
          day = day + set;
          day = Binary2BCD(day);
          if (day >= 0x32)
            day = 1;
          if (day <= 0)
            day = 0x31;
          write_ds1307(4, day); // write date 17
          break;
        case 5:
          month = BCD2Binary(month);
          month = month + set;
          month = Binary2BCD(month);
          if (month > 0x12)
            month = 1;
          if (month <= 0)
            month = 0x12;
          write_ds1307(5, month); // write month 6 June
          break;
        case 6:
          year = BCD2Binary(year);
          year = year + set;
          year = Binary2BCD(year);
          if (year <= -1)
            year = 0x99;
          if (year >= 0x50)
            year = 0;
          write_ds1307(6, year); // write year
          break;
        }
      }
    }

    // RTC
    second = read_ds1307(0);
    minute = read_ds1307(1);
    hour = read_ds1307(2);
    hr = hour & 0b00011111;
    ap = hour & 0b00100000;
    dday = read_ds1307(3);
    day = read_ds1307(4);
    month = read_ds1307(5);
    year = read_ds1307(6);

    time[0] = BCD2UpperCh(hr);
    time[1] = BCD2LowerCh(hr);
    time[3] = BCD2UpperCh(minute);
    time[4] = BCD2LowerCh(minute);
    time[9] = BCD2UpperCh(second);
    time[10] = BCD2LowerCh(second);

    date[0] = BCD2UpperCh(month);
    date[1] = BCD2LowerCh(month);
    date[3] = BCD2UpperCh(day);
    date[4] = BCD2LowerCh(day);
    date[6] = BCD2UpperCh(year);
    date[7] = BCD2LowerCh(year);

    if (ap) {
      time[6] = 'P';
      time[7] = 'M';
    } else {
      time[6] = 'A';
      time[7] = 'M';
    }

    distance = getDistance();
    percentage = map(distance, 90, 10, 0, 100); // for 20cm tank
    if (percentage <= 0) {
      percentage = 0;
    } else if (percentage >= 100) {
      percentage = 100;
    }

    IntToStr(percentage, distanceString);
    ltrim(distanceString);
    rtrim(distanceString);
    //Delay_ms(500);
    UART1_Write('x');
    for (k = 0; k < 4; k++) {
      UART1_Write(distanceString[k]);
    }

    UART1_Write('\n');
    UART1_Write('\r');
    timer1_delay(500);

    if (time[0] != '0') {
      UART1_Write('t');
      for (m = 0; m < 8; m++) {
        UART1_Write(time[m]);
      }

      UART1_Write('\n');
      UART1_Write('\r');
      timer1_delay(500);
    }

    if (time[0] == '0') {
      UART1_Write('t');
      for (m = 1; m < 8; m++) {
        UART1_Write(time[m]);
      }

      UART1_Write('\n');
      UART1_Write('\r');
      timer1_delay(500);
    }

    //if (((time[9] == '0') && (time[10] == '0')) || ((time[9] == '3') && (time[10] == '0'))) {
    UART1_Write('d');
    for (i = 0; i < 8; i++) {
      UART1_Write(date[i]);
    }
    UART1_Write('\n');
    UART1_Write('\r');
    timer1_delay(500);
    //}

    if (uart_rd == 't') {
      LATD.F1 = 0;
    }

    if (uart_rd == 'T') {
      LATD.F1 = 1;
    }

    /*if (percentage <= 50) {
      LATD.F1 = 0;
    } else if ((percentage > 70) && (percentage <= 80)) {
      LATD.F1 = 1;
    }*/

    //UART1_Write('o');
    //for (l = 0; l < 2; l++) {
    /*if (SoftSerial.GetChar() == 'a') {
      UART1_Write('a');
      Delay_ms(50);
      UART1_Write('o');
    }

    if (SoftSerial.GetChar() == 'b') {
      UART1_Write('b');
      Delay_ms(50);
      UART1_Write('o');
    }

    if (SoftSerial.GetChar() == 'c') {
      UART1_Write('c');
      Delay_ms(50);
      UART1_Write('o');
    }

    if (SoftSerial.GetChar() == 'd') {
      UART1_Write('d');
      Delay_ms(50);
      UART1_Write('o');
    }

    if (SoftSerial.GetChar() == 'e') {
      UART1_Write('e');
      Delay_ms(50);
      UART1_Write('o');
    }

    if (SoftSerial.GetChar() == 'f') {
      UART1_Write('f');
      Delay_ms(50);
      UART1_Write('o');
    } */
    //}
  }
}