/*
TideWeatherStation

 16x2 LCD display.  The LiquidCrystal
 library works with all LCD displays that are compatible with the 
 Hitachi HD44780 driver. There are many of them out there, and you
 can usually tell them by the 16-pin interface.
 
 This sketch displays text sent over the serial port 
 (e.g. from the Serial Monitor) or rx on pin 0 on an attached LCD.
 
 Need to read the luminosity and the barometer
 
See APRS101 doc top of page 95 for Lat Long position report format with data ext ad timestamp
 
 The circuit:
 * LCD RS pin (pin 4 on the lcd) to digital pin 8  
 * LCD Enable pin (pin 6 on the lcd) to digital pin 9 
 * LCD D4 pin (pin11 on the lcd) to digital pin 4
 * LCD D5 pin (pin 12 on the lcd) to digital pin 5 
 * LCD D6 pin (pin 13 on the lcd) to digital pin 6 
 * LCD D7 pin (pin 14 on the lcd) to digital pin 7 
 * LCD R/W pin (pin 5 on the lcd) to ground 
 * 10K resistor:
 * ends to +5V and ground
 * wiper to LCD VO pin (pin 3) 
 
 Library originally added 18 Apr 2008
 by David A. Mellis
 library modified 5 Jul 2009
 by Limor Fried (http://www.ladyada.net)
 example added 9 Jul 2009
 by Tom Igoe 
 modified 22 Nov 2010
 by Tom Igoe
 modified Doug Fredericks Feb 2012
 
 For Barometer
 
 *************************************************** 
  This is an example for the BMP085 Barometric Pressure & Temp Sensor

  Designed specifically to work with the Adafruit BMP085 Breakout 
  ----> https://www.adafruit.com/products/391

  These displays use I2C to communicate, 2 pins are required to  
  interface
  Adafruit invests time and resources providing this open source code, 
  please support Adafruit and open-source hardware by purchasing 
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.  
  BSD license, all text above must be included in any redistribution
 ****************************************************

Connect VCC of the BMP085 sensor to 3.3V (NOT 5.0V!)
Connect GND to Ground
Connect SCL to i2c clock - on '168/'328 Arduino Uno/Duemilanove/etc thats Analog 5
Connect SDA to i2c data - on '168/'328 Arduino Uno/Duemilanove/etc thats Analog 4
EOC is not used, it signifies an end of conversion
XCLR is a reset pin, also not used here

For the Sonar Distance Measuring, use Serial
gnd to gnd
5v to 5v
Arduino Pin 0 (serial rx) to Sensor pin tx
Arduino digital Pin 2 to Sensor pin rx 
 
 */

// include the library code for Display:
#include <LiquidCrystal.h>
#include <stdlib.h>
// Temp and Barom
#include <Wire.h>                       
#include <Adafruit_BMP085.h>
// Serial tools since using more than pins 0, 1.  Use pin 10 to rx the gps
#include <SoftwareSerial.h>
SoftwareSerial mySerial = SoftwareSerial(10, 11); // RX, TX  Not using TX 11, so can I just leave it off the list?
//
// initialize the library with the numbers of the interface pins, same order as description above
 LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
 int k;
 int i;
 int j;
 int n;
//
 char linea[300] = "";  // for the GPS packet
 char lineGPS[300] = "";
 char GPSpacket[68] = ""; 
 char comandoGPR[7] = "$GPRMC";
 char GPSholder = 65;
 int byteGPS=-1;
 int cont=0;
 int bien=0;
 int pcount=0;
 int conta=0;
 int indices[19];
 int periodLoc[9];
 char TMP[4]; 
 char MSGFLAG = '/'; //  slash is no msg, @ is yes msg
 char WARN = 'V';  // for warning A or V
 char ZULU = 'z' ;  // needed after HMS
 char SYMBOLTABLEID = '/'; // slash is primary table 
 char SYMBOLCODE='w'; // Water station
 char COM[36]="Put comment here"; // Max 36 char comment
//
 char PTMP[3];        // for assembling the packet of 3 integers from TMP
 char BAROM[10];
 char TID[10];
 char LUM[4];
// 
 char PLUM[3];       // for assembling the packet of 3 integers from LUM
// 
 char packet[45];    // This is the ultimate packet sent to the TNC
 // flags
 int flagRadioTransmit;
// 
//
// Items needed for barometer and temperature
//
 Adafruit_BMP085 bmp;   // For Temp and Barom 
 int t;   // temp in F
 float b;   // barometric pressure
 //
 // Items needed for photocell 
 //
 int photocellPin = 1;     // the cell and 10K pulldown are connected to a1
 int photocellReading;     // the analog reading from the sensor divider
 //
 // Items needed for sonar
 //
 int byteSonar=-1;
 char lineb[5] = "";
 char templine[10] = "";
 int countb;
 float datum=10.0;
 float snr;    // Datum - sonar reading
 float f;      // tide after subtracting the datum
 
void setup(){
  //
  // set up the LCD's number of columns and rows:
  pinMode(13, OUTPUT);   
  pinMode(3, OUTPUT);   
  // byteGPS=-1;
  // byteSonar=-1;
  //
  lcd.begin(16, 2);
  lcd.print("Hi Doug");  //so i know it is working
  // initialize the serial communications:
  Serial.begin(9600);                                                         
  mySerial.begin(4800);                      // Surprised that you can do different speeds, cool.
  // initialize I2C with temp and barom
  bmp.begin();   
}

void loop() {
  flagRadioTransmit = 1;    // 0 off, 1 on
  //
  digitalWrite(13, LOW);
  digitalWrite(3, LOW);
  lcd.clear();
  delay(30);
  //
  // Read the GPS from the device attached to Arduino Pin 10  ********************************************************************************************
  //
  byteGPS=-1;
  byteGPS=mySerial.read();        // Read a byte of the serial port                     
  if (byteGPS == -1) {            // See if the port is empty and if so skip reading gps
     lcd.print("No Recd GPS");
     delay(1000);                                                                     // will want a flag for this to turn it off outside of debugging
  } else {
     lcd.println("Recd GPS ");
     lcd.setCursor(0,1);
    // itoa(byteGPS,GPSholder,2);
    // lcd.print(byteGPS);
     delay(1000); 
//     linea[conta]=byteGPS;         // If there is serial port data, it is put in the buffer               
//     conta++;                 
//
     for (i=1;i<300;i++){ 
         byteGPS=mySerial.read();
    //      lcd.print(byteGPS);
    //      delay(100);    
         lineGPS[i] = byteGPS;
    //     lcd.print(lineGPS[i]);   
     }  
     for (i=1;i<250;i++){     
       if (lineGPS[i]=='G'){
         if (lineGPS[i+1]=='P') {
           if (lineGPS[i+2]=='R') {
             if (lineGPS[i+3]=='M') {
               if (lineGPS[i+4]=='C') {
//               get northing
                 packet[1]=lineGPS[i+15];
                 packet[2]=lineGPS[i+16];
                 packet[3]=lineGPS[i+17];
                 packet[4]=lineGPS[i+18];
                 packet[5]=lineGPS[i+19];
                 packet[6]=lineGPS[i+20];
                 packet[7]=lineGPS[i+21];
                 packet[8]=lineGPS[i+25];
//               get easting 
                 packet[10]=lineGPS[i+27];
                 packet[11]=lineGPS[i+28];
                 packet[12]=lineGPS[i+29];
                 packet[13]=lineGPS[i+30];
                 packet[14]=lineGPS[i+31];
                 packet[15]=lineGPS[i+32];
                 packet[16]=lineGPS[i+33];
                 packet[17]=lineGPS[i+34];
                 packet[18]=lineGPS[i+35];
//               note not filling in ! or the slash and the underline on packet9 and packet19
               }
             }
           }
         }
       }
     } 
     //    k=26;
//       lcd.print(SYMBOLCODE); //Symbol Code from void set above
     //    GPSpacket[k]=SYMBOLCODE;
         //
//         }
         delay(500);
         lcd.clear();
         for (i=8;i<24;i++){                  
          lcd.print(packet[i]);
          }
         lcd.setCursor(0,1);
         for (i=0;i<7;i++){                  
          lcd.print(packet[i]);
         }
         delay(1000);
         lcd.clear();
         for (i=27;i<34;i++){                  
          lcd.print(packet[i]);
         }
         delay(2000);   //  Need to have smart beacon here
         //  Yes! GPSPacket is complete!
      }           
       conta=0;                    // Reset the buffer
       for (i=0;i<300;i++){    //  
         lineGPS[i]=' ';             
       }                 
   lcd.clear(); 
// 
// Do the photocell *******************************************************************************************************************
//
  photocellReading = analogRead(photocellPin);  // reads an integer off the pin that could be 1, 2 or 3 digits
  if (photocellReading > 99) 
     {
     String LUM = String(photocellReading);
//     lcd.print("Lumino>99 ");
//     lcd.print(LUM);
//     lcd.print(" ");
//     lcd.print(LUM[2]);
     PLUM[0] = LUM[0];
     PLUM[1] = LUM[1];
     PLUM[2] = LUM[2];
     }
    else if (photocellReading < 10)
     {
     String LUM = String(photocellReading);
     PLUM[0] = '0';
     PLUM[1] = '0';
     PLUM[2] = LUM[0];
     }
     else
     {
     String LUM = String(photocellReading);
     PLUM[0] = '0';
     PLUM[1] = LUM[0];
     PLUM[2] = LUM[1];
     }
  lcd.print("Lumino= ");
  lcd.print(PLUM[0]);
  lcd.print(PLUM[1]); 
  lcd.print(PLUM[2]);
  //    lcd.print(" S");  
  lcd.setCursor(0,1);
  lcd.print(photocellReading);
  //
  delay(1000);
  lcd.clear(); 
  //
  // Do the temp and barometer reading *******************************************************************************************************************
  //
  lcd.print("Temp = ");
  //  lcd.print(round(bmp.readTemperature()*9.0/5.0+32.0));
  //  lcd.print(" *F");
  //  lcd.setCursor(0,1);
  t = int(bmp.readTemperature()*9.0/5.0+32.0);
  //  dtostrf(t,3, 0, TMP); // float to string in stdlib is dtostrf(FLOAT,WIDTH,PRECSISION,BUFFER);
    if (t > 99) 
     {
     String TMP = String(t);
     PTMP[0] = TMP[0];
     PTMP[1] = TMP[1];
     PTMP[2] = TMP[2];
     }
    else if (t < 10)
     {
     String TMP = String(t);
     PTMP[0] = '/0';
     PTMP[1] = '/0';
     PTMP[2] = TMP[1];
     }
    else 
     {
     String TMP = String(t);
     PTMP[0] = '/0';
     PTMP[1] = TMP[0];
     PTMP[2] = TMP[1];
     }
  lcd.print(PTMP[0]);
  lcd.print(PTMP[1]); 
  lcd.print(PTMP[2]);
  lcd.print(" *F");
  lcd.setCursor(0,1);
  //
  lcd.print("Pres= ");
  // divide Pa by 100 to bet hPa which is also mb then divide by adjustment factor
  // lcd.print((bmp.readPressure()/100.0)-0);
  // lcd.println(" mb ");
  b=bmp.readPressure()/100.0-0;
  dtostrf(b,4, 1, BAROM); // float to string in stdlib is dtostrf(FLOAT,WIDTH,PRECSISION,BUFFER);
  lcd.print(BAROM);
  lcd.println(" mb ");
  delay(1000);
  lcd.clear();
  //
  // Do the sonar distance measuring  *******************************************************************************************************************
  //
  digitalWrite(3, HIGH);
  delay(200); 
    lineb[0]='n';
    lineb[1]='o';
    lineb[2]='b';
    lineb[3]='i';
    lineb[4]='t';
//  
  byteSonar=-1;
  byteSonar=Serial.read();        // Read a byte of the serial port                        
  if (byteSonar==-1){
    lcd.println("no sonar");
    delay(1000);
  } else {
    lcd.println("got sonar");    
    delay(1000);
    lcd.clear();
    for (i=1;i<10;i++){    
      templine[i-1]=Serial.read();
      lcd.print(templine[i]);
    }
    lcd.setCursor(0,1);
    delay(2000);
    lineb[0]=templine[4];
    lineb[1]=templine[5];
    lineb[2]=templine[6];
    lineb[3]='0';
    lineb[4]='0';
    lineb[5]='0';
    } 
    lcd.print("  ");
    lcd.print(lineb[0]);
    lcd.print(lineb[1]);
    lcd.print(lineb[2]);
    lcd.print(lineb[3]);
    lcd.print(lineb[4]);
    lcd.print(lineb[5]);
    delay(2000);
//
// Do the datum math
//
    // readstring.toCharArray(lineb, 5); //put readStringinto an array
    snr = atof(lineb); //convert the array into a Float
    f = datum - snr;
//
    lcd.clear();
    digitalWrite(3, LOW);
  //
  // Assemble Packet  *******************************************************************************************************************
  //
  //   this was pulled from a raw pkt from a weather station   !3839.35N/12120.19W_232/002g006t072r000P000p000h51b10138.VWS-DavisVVue   
  // This worked   String  packet[45] = "!3812.43N/12234.14W_000/000t086b10065L340F+030" ; at least on the th-d72a did not try to digi it
  //                  012345678901234567890123456789012345678901234567890123456789
  //                  0         1         2         3         4         5
  // This worked   char packet[68] = "!3812.43N/12234.14W_000/000t086L340b10065F+030" ;
  // Page 65 of aprs protocol version 1.0, t=056 deg F, Lumin=320, barom=1008.5, Flood=-3.5
  // Has Petaluma coords, need to overwrite data after the wind and gust
  //
//  char packet[] = {'!', '3','8','1','2','.','4','3','N','/','1','2','2','3','4','.','1','4','W','_','.','.','.','/','.','.','.','t','0','8','8','b','1','0','0','7','5','L','3','4','0','F','+','0','3','0'} ;  
  //                  0    1   2   3   4   5   6   7   8   9   0   1   2   3   4   5   6   7   8   9   0   1   2   3   4   5   6   7   8   9   0   1   2   3   4   5   6   7   8   9   0   1   2   3   4   5   67890123456789
  //                  0                                        1                                       2                                       3                                       4                           5
//       lcd.print(SYMBOLTABLEID); //Symbol Table ID from void set above  Need sim for the ! and the _
  packet[0]='!';
  packet[9]=SYMBOLTABLEID;
  packet[19]='_';
  packet[20]='.';
  packet[21]='.';
  packet[22]='.';
  packet[23]='/';
  packet[24]='.';
  packet[25]='.';
  packet[26]='.';
  packet[27]='t';
  packet[28] = PTMP[0] ;
  packet[29] = PTMP[1] ;
  packet[30] = PTMP[2] ;
  packet[31] = 'b' ;
  packet[32] = BAROM[0] ;
  packet[33] = BAROM[1] ;
  packet[34] = BAROM[2] ;  
  packet[35] = BAROM[3] ;
  packet[36] = BAROM[5] ;
  packet[37] = 'L' ;
  packet[38] = PLUM[0] ;  
  packet[39] = PLUM[1] ;  
  packet[40] = PLUM[2] ;
  packet[41] = 'F' ;
  packet[42] = '+';
  packet[43] = '+' ;
  packet[44] = '+' ;
  packet[45] = '+' ;
  //
  //
  // Flag it on or off for testing
  //
  if (flagRadioTransmit = 0) {
    digitalWrite(13, HIGH);
    delay(10);
    for (i=0;i<45;i++){    // Remember to increase this to packet size!   
      Serial.print(packet[i]);
    }
    Serial.println("");
    delay(2000);
    digitalWrite(13, LOW);
     //  lcd.println("Just Sent Packet");
    for (i=22;i<38;i++){    
      lcd.print(packet[i]);
    }
    lcd.setCursor(0,1);
    for (i=37;i<46;i++){     
      lcd.print(packet[i]);
      }
    delay(30000);
    } else {
    lcd.println("RadioSendFlagOff");   
    delay(2000);  
    }
 }


