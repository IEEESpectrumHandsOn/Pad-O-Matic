/*------------------------------------------------------------------------
  One Time Pad Machine.

  Reads an 8-bit random number from a noise source that has been digitised
  and debiased. Then throws away byte if it is greater than 250, and does
  modular division by 10 on any other bytes to get digits from 0-9. 

  The digits are then used to create one time pads with 250 digits in each,
  grouped in blocks of five digits. digits are stored then sent to a thermal 
  printer to be printed twice, one copy for each user. 

  To see the accompanying IEEE Spectrum Hands On article, visit:

  https://spectrum.ieee.org/diy-one-time=pad-machine

  IEEE Spectrum is cool. IFYKYK. Geeks get it, you should too!

  Microcontroller target: Uno R4

  Requires the Adafruit Adruino Library for Small Thermal Printers: an oldie but still a goodie.
  Install from Arduino library manager or get from here: 

  https://github.com/adafruit/Adafruit-Thermal-Printer-Library

  This code incorporates material from the thermal printer library example. Additional
  code licensed under MIT License.

  MIT License

  Copyright (c) [2025] [Stephen Cass/IEEE Spectrum]

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
  
  ------------------------------------------------------------------------*/

#include "Adafruit_Thermal.h" 

#include "SoftwareSerial.h"
#define TX_PIN 2 // Arduino transmit  YELLOW WIRE  labeled RX on printer
#define RX_PIN 3 // Arduino receive   GREEN WIRE   labeled TX on printer

#define BYTE_AVAIL 13 // When number generator brings this high, random byte is ready
#define BYTE_READ 12 // Bring this ping low to enable latch and read result
#define BIT_SEVEN 11 //Can't use AVR ports on Rev 4, so we'll be bit banging
#define BIT_SIX 10
#define BIT_FIVE 9
#define BIT_FOUR 8
#define BIT_THREE 7
#define BIT_TWO 6
#define BIT_ONE 5
#define BIT_ZERO 4

#define GO_BUTTON 14 //Use A0 as digital input to read control button.
#define STATUS_LED 15 //Use A1 to blink when generating code book

#define PAD_TOTAL 50 // How many pads of 250 characters each we have per code book. With printing 4 code books, for each side to have IN and OUT pad, this is what will fit on ~50' roll.

#define DIAGNOSTICS 0 //Really should have made this a run-time sense switch option, but set this at compile time to have the Pad-O-Matic generate 1M digits and print statistics for each

byte codebook[(PAD_TOTAL*250)];
int code_ok;
int total_digits;
int digit_count[] = {0,0,0,0,0,0,0,0,0,0};
byte diag_digit;
int toggle = 0;

SoftwareSerial mySerial(RX_PIN, TX_PIN); // Declare SoftwareSerial obj first
Adafruit_Thermal printer(&mySerial);     // Pass addr to printer constructor

void setup() {

  pinMode(BYTE_AVAIL, INPUT);
  pinMode(BYTE_READ, OUTPUT);
  digitalWrite(BYTE_READ, HIGH);

  pinMode(BIT_SEVEN, INPUT);
  pinMode(BIT_SIX, INPUT);
  pinMode(BIT_FIVE, INPUT);
  pinMode(BIT_FOUR, INPUT);
  pinMode(BIT_THREE, INPUT);
  pinMode(BIT_TWO, INPUT);
  pinMode(BIT_ONE, INPUT);
  pinMode(BIT_ZERO, INPUT);

  pinMode(GO_BUTTON, INPUT_PULLUP);

  pinMode(STATUS_LED, OUTPUT);
  digitalWrite(STATUS_LED, LOW);
  
  digitalWrite(BYTE_READ, HIGH);

  total_digits = PAD_TOTAL*250;
  
  // NOTE: SOME PRINTERS NEED 9600 BAUD instead of 19200, check test page.
  mySerial.begin(19200);  // Initialize SoftwareSerial
  //Serial1.begin(19200); // Use this instead if using hardware serial
  printer.begin();        // Init printer (same regardless of serial type)
  
}

void loop() {

  while(digitalRead(GO_BUTTON) == HIGH){
    //wait for go
  }

  if (DIAGNOSTICS) {
    printer.println("Diagnostics Mode");
    for(int i=0; i < 1000000; i++) {
      diag_digit = int(get_random_digit());
      digit_count[diag_digit] =  digit_count[diag_digit]+1; 

      if ( toggle == 1 ) {
        digitalWrite(STATUS_LED, HIGH);
        toggle = 0;
      }
      else {
        digitalWrite(STATUS_LED, LOW);
        toggle = 1;
      }
   }
    digitalWrite(STATUS_LED, LOW); //With the hardware RNG, the blink per digit will be too fast to see, but it's a handy diagnostic.
   for(int i=0; i<10; i++) {
      printer.print(i);
      printer.print("--");
      printer.println(digit_count[i]);
   }
  
   while(true);
    
  }

  for(int k=0; k<2; k++) { // go round twice to create one copy for Alice and one for Bob

    code_ok = create_book(); // fills the codebook array with those tasty random digits.
    if (code_ok == 0 ) {
     printer.println("Error generating code book"); //A hook for error conditions I ended up not using
     //continue;
    }
  
   printer.setFont('A');
   printer.justify('C'); //center printer output//

    for(int l=0; l<2; l++) { // go round twice to create in and out books
    
     printer.println("*****COPY STARTS*****");
   
     for(int i=0; i<PAD_TOTAL; i++) {
       printer.println("--------------------");
       if(l == 0) {
         printer.println("--IN--");
        }
        else {
         printer.println("--OUT--");
       }
      
        printer.println("Tear off and burn after use."); // Reinforce to user that "one time" means ONE TIME.
        printer.println("");
  
      for(int j=0; j<250; j++){
    
        if ( j % 5 == 0 ) {
         printer.print(" "); 
        }
        if ( j % 25 == 0 ) {
          printer.println("");
        }
      
       printer.print(codebook[j+(i*250)]);
       }
  
     printer.println(" ");
     printer.println("--------------------");
    }

    printer.println("*****COPY ENDS*****");

    printer.println("--Conversion Table--"); //For convenience. 98 is often used to mean "QUERY?" as well as "?" in other tables
    printer.println("Code-0  B-70  P-80  FIG-90");
    printer.println("   A-1  C-71  Q-81  (.)-91");
    printer.println("   E-2  D-72  R-82  (:)-92");
    printer.println("   I-3  F-73  S-83  (')-93");
    printer.println("   N-4  G-74  U-84  ( )-94");
    printer.println("   O-5  H-75  V-85  (+)-95");
    printer.println("   T-6  J-76  W-86  (-)-96");
    printer.println("        K-77  X-87  (=)-97");
    printer.println("        L-78  Y-88  (?)-98");
    printer.println("        M-79  Z-89  SPC-99");
    printer.println(" ");
    printer.println("---------------------");
    printer.println("CUT HERE");
    printer.println("---------------------");
    printer.println("");
  
  }
  }
}

int create_book() {
  for(int i=0; i < total_digits; i++){
    codebook[i] = get_random_digit();
    if ( toggle == 1 ) {
      digitalWrite(STATUS_LED, HIGH);
      toggle = 0;
    }
    else {
            digitalWrite(STATUS_LED, LOW);
      toggle = 1;
      }
    }
  digitalWrite(STATUS_LED, LOW);
  return 1; //hook here for error codes
}

byte get_raw_byte() {
  
  byte raw_byte = 0;
  // return byte(random(256)); // code for testing without external entropy hardware ONLY. Also if you uncomment this, discover just how slow the software RNG really is, e.g. you'll be able to see the status LED blinking!

  // Wait for signal there is an available byte. It's not like we have anything else to do
  while( digitalRead(BYTE_AVAIL) == LOW ) {
  }

  //request byte to be presented on RNG hardware output by bringing read byte low 
  digitalWrite(BYTE_READ, LOW);

  //read bits. I'm sure there's a better way to this, but this works fine. 
  raw_byte = digitalRead(BIT_ZERO);
  raw_byte = raw_byte + ( digitalRead(BIT_ONE) << 1 ); //shift each bits into its proper place in the random byte
  raw_byte = raw_byte + ( digitalRead(BIT_TWO) << 2 );
  raw_byte = raw_byte + ( digitalRead(BIT_THREE) << 3 );
  raw_byte = raw_byte + ( digitalRead(BIT_FOUR) << 4 );
  raw_byte = raw_byte + ( digitalRead(BIT_FIVE) << 5 );
  raw_byte = raw_byte + ( digitalRead(BIT_SIX) << 6 );
  raw_byte = raw_byte + ( digitalRead(BIT_SEVEN) << 7 );
  
  //release hardware read so it can work on next digit
  digitalWrite(BYTE_READ, HIGH);
  
  return raw_byte;
}

byte get_random_digit() {
  byte raw_byte;
  byte code_digit;
  raw_byte = 255;
  while(raw_byte > 250 ) { //loop till we get a random byte less than 251 so we don't get an unbalanced distribution when do mod 10 division below
    raw_byte = get_raw_byte();
  }
  
  //Turn that raw byte into a digit 0-9 via maodular division.
  code_digit = raw_byte % 10;

  return code_digit;  
  }
