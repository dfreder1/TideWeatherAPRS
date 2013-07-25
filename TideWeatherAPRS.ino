/*
TideWeatherStation
   
See APRS101 doc for APRS msg details, such as the 
top of page 95 for Lat Long position report format with data ext and timestamp
 
LCD:
 For debugging only, use a 16x2 LCD display.  The LiquidCrystal
 library works with all LCD displays that are compatible with the 
 Hitachi HD44780 driver. There are many of them out there, and you
 can usually tell them by the 16-pin interface.

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
 
 LCD Library originally added 18 Apr 2008
 by David A. Mellis
 library modified 5 Jul 2009
 by Limor Fried (http://www.ladyada.net)
 example added 9 Jul 2009
 by Tom Igoe 
 modified 22 Nov 2010
 by Tom Igoe
 modified Doug Fredericks Feb 2012
 
For Barometer:
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

Sonar:
 For the Sonar Distance Measuring, use Serial
 gnd to gnd
 5v to 5v  NOT using this as instead using digital pin 2 to power the sonar
 Arduino Pin 0 (serial rx) to Sensor pin tx
 Arduino digital Pin 3 to Sensor +5v power                                       
 
*/

// include the library code for LCD Display:
#include <LiquidCrystal.h>
#include <stdlib.h>
// Temp and Barom    Wait is this really necessary?
#include <Wire.h>                       
// Memory for the GPS readings so you can disconnect the GPS after a minute or so
#include <EEPROM.h>                       
// For the temperature and barometer
#include <Adafruit_BMP085.h>
// Serial tools since using more than pins 0, 1.  Use pin 10 to rx the gps
#include <SoftwareSerial.h>
SoftwareSerial mySerial = SoftwareSerial(10, 11); // RX, TX  Not using TX 11, so can I just leave it off the list?
//
// For LCD initialize the library with the numbers of the interface pins, same order as description above
 LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
 int k;
 int i;
 int j;
 int z;
 int n=0;   // used to count the number of times looped thru and set the serial number on telemetry packet
 int tt1=1;   // used to get the fake tide for tweeting
 int tt2=1;
//
 char linea[300] = "";  // for the GPS packet
 char lineGPS[300] = "";
// char comandoGPR[7] = "$GPRMC";
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
 //
 char LUM[4];
 char PLUM[3];       // for assembling the packet of 3 integers from LUM
 //
 char NNN[4];
 char SER[3];       // for assembling the packet of 3 integers from n
 char TTT[3];       // for tweeting a "tide"
// 
 char packet[46];    // This is the ultimate packet sent to the TNC for the location and weather. 0-44 info 45 null so 46 total
 char telempkt[36] ;   // This is the telemetry packet with sonar (water height) data
 char tweetpkt[30];    // 0 to 28 is info and 29 is null, so 30 total 
 char testpacket[47];  // info is contained in 0 to 45, which is 46 addresses, 47th address is for the null character
 // flags
 int flagTestTransmit = 1;  // initialize here but change values below
 int flagRadioTransmitWx = 1;
 int flagRadioTransmitTelem = 1;
 int flagRadioTransmitTweet = 1;
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
 char lineb[3] = "";
 char templine[100] = "";
 int countb;
 float datum=10.0;  // not using this
 float snr;    // not using this     Datum - sonar reading
 float f;      // not using this     tide after subtracting the datum
 
void setup(){
  //
  pinMode(13, OUTPUT);  // radio enable   
  pinMode(3, OUTPUT);   // sonar enable
  //
  lcd.begin(16, 2);
  lcd.print("Hi Doug");  //so i know it is working
  // initialize the serial communications:
  Serial.begin(9600);                        // read from sonar into pin 0, transmit from pin 1 to tinypack                                 
  mySerial.begin(4800);                      // gps Surprised that you can do different speeds, cool.
  // initialize I2C with temp and barom
  bmp.begin();   
}

void loop() {
<<<<<<< HEAD
  flagTestTransmit = 0;
=======
  flagTestTransmit = 0;     // 0 is radio tx off, 1 is tx on
>>>>>>> newfeature
  flagRadioTransmitWx = 0;    // 0 is radio tx off, 1 is tx on
  flagRadioTransmitTelem = 0;    // 0 is radio tx off, 1 is tx on
  flagRadioTransmitTweet = 1;    // 0 is radio tx off, 1 is tx on
  n = n+1; // start the counter
  //
  digitalWrite(13, LOW);  // 13 high triggers radio transmit
  digitalWrite(3, LOW);   // 3 high triggers sonar 
  lcd.clear();
  delay(30);
  //
  // Read the GPS from the device attached to Arduino Pin 10  ********************************************************************************************
  //
  byteGPS=-1;
  byteGPS=mySerial.read();        // Read a byte of the serial port                     
  if (byteGPS == -1) {            // See if the port is empty and if so skip reading gps
     lcd.print("No Recd GPS");
     delay(1000);                                                    
  } else {
     lcd.println("Recd GPS ");
     lcd.setCursor(0,1);
     delay(500); 
//     linea[conta]=byteGPS;         // If there is serial port data, it is put in the buffer               
//     conta++;                 
//
     for (i=1;i<100;i++){ 
         lineGPS[i]=mySerial.read();
     }    
    //    
    //  example  $GPRMC,081836,A,3751.6115,S,14507.3226,E,000.0,then other stuff im not using
    //           12345678901234567890123456789012345678901234567890123456789012334567890
    //           0        1         2         3         4         5         6
    //           This is South latitude 37d51.6115m, East longitude 145d07.3226m    
    //           You can delete this line, just added to test git 
     for (i=0;i<50;i++){     
       if (lineGPS[i]=='G'){
         if (lineGPS[i+1]=='P') {
           if (lineGPS[i+2]=='R') {
             if (lineGPS[i+3]=='M') {
               if (lineGPS[i+4]=='C') {
//               if we got here, then we have the gprmc sentence starting at i and for some reason i is usu but not always 1
//               doesnt matter what i is  
//               get northing
                 packet[1]=lineGPS[i+15];   // first digit of the Northing so prob a 3
                 packet[2]=lineGPS[i+16];
                 packet[3]=lineGPS[i+17];   
                 packet[4]=lineGPS[i+18];
                 packet[5]=lineGPS[i+19];  // should be a period
                 packet[6]=lineGPS[i+20];
                 packet[7]=lineGPS[i+21];
//                 packet[8]=lineGPS[i+22];  can't packet these digits to aprs
//                 packet[9]=lineGPS[i+23];
                 packet[8]=lineGPS[i+25]; // this will be N or S
//               get easting 
                 packet[10]=lineGPS[i+27];  // should be first digit of Easting
                 packet[11]=lineGPS[i+28];
                 packet[12]=lineGPS[i+29];
                 packet[13]=lineGPS[i+30];
                 packet[14]=lineGPS[i+31];
                 packet[15]=lineGPS[i+32];   // should be a period
                 packet[16]=lineGPS[i+33];
                 packet[17]=lineGPS[i+34];
//                 packet[20]=lineGPS[i+35];  can't packet these digits to aprs
//                 packet[21]=lineGPS[i+36];
                 packet[18]=lineGPS[i+38];  // will be E or W
//               note not filling in ! or the slash and the underline on packet9 and packet19 will do that below
                // 
                // If these are new then store in memory, pick an offset of 100 for mem location burn cycles protection
                //
                  for (j=1;j<19;j++){   
                    if (packet[j]==int(EEPROM.read(100+j))) {                      
                      Serial.print(j);
                      Serial.println(" Old val same as new, so not stored ");
                    } else {
                      EEPROM.write(100+j,packet[j]);
                      Serial.print(j);
                      Serial.println(" New val diff than old, so is stored ");
                    }
                      //Serial.print(int(lineGPS[j]));
                      //Serial.println(" same? ");
                  }                    
               }
             }
           }
         }
       }
     } 
    }           
       for (i=0;i<300;i++){    //  Reset the buffer
         lineGPS[i]=' ';             
       }    
//  print to lcd what it thinks the lat lon is, note that it really uses the eeprom not the packet at this point so this could actually read bad values 
         delay(500);
         lcd.clear();
         for (i=1;i<8;i++){                  
          lcd.print(packet[i]);
         }
         lcd.setCursor(0,1);
         for (i=10;i<18;i++){                  
          lcd.print(packet[i]);
         }
         delay(1000); 
   lcd.clear(); 
// 
// Do the photocell *******************************************************************************************************************
//
  photocellReading = analogRead(photocellPin);  // reads an integer off the pin that could be up to 1023
  photocellReading = map(photocellReading, 0, 1023, 0, 255); // aprs telemetry can only take up to 255
  if (photocellReading > 99) 
     {
     String LUM = String(photocellReading);
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
  lcd.print(photocellReading);
//  lcd.print(analogRead(photocellPin));
  lcd.setCursor(0,1);
  lcd.print(PLUM[0]);
  lcd.print(PLUM[1]); 
  lcd.print(PLUM[2]);
  //    lcd.print(" S");  
  // lcd.setCursor(0,1);
  // lcd.print(photocellReading);
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
  lcd.print(b); 
  // lcd.print(" ");
  lcd.print(BAROM);
  lcd.println(" mb ");
  delay(1000);
  lcd.clear();
  //
  // Do the sonar distance measuring  *******************************************************************************************************************
  //
  digitalWrite(3, HIGH);   // seems to be that I have to cycle through it twice
  delay(1000);
     for (i=0;i<99;i++){    
     templine[i]=Serial.read();
     // lcd.print(templine[i]);
     } 
  delay(1000); 
    lineb[0]='n';
    lineb[1]='o';
    lineb[2]='b';
//  
  byteSonar=-1;
  byteSonar=Serial.read();        // Read a byte of the serial port                        
  if (byteSonar==-1){
    lcd.println("no sonar");
    delay(500);
  } else {
    lcd.println("got sonar");    
    delay(1000);
    lcd.clear();
    for (i=0;i<99;i++){    
      templine[i]=Serial.read();
     // lcd.print(templine[i]);
    }
  //      for (i=0;i<99;i++){    // Remember to change this to packet size and to not use testpacket!   
  //      Serial.print(templine[i]);
   //     }
   //     Serial.println("");
   //     delay(1000);
    lcd.setCursor(0,1);
    delay(500);
    lineb[0]=templine[50];  // just used trial and error to get the last readout in templine which is the most recent
    lineb[1]=templine[51];
    lineb[2]=templine[52];
    } 
    digitalWrite(3, LOW);
    lcd.print("  ");
    lcd.print(lineb[0]);  // distance from sonar to water in inches - 028 is 28"
    lcd.print(lineb[1]);
    lcd.print(lineb[2]);
    delay(2000);
//
//
    lcd.clear();
  //
  // Assemble Weather Packet  *******************************************************************************************************************
  //
  //   this was pulled from a raw pkt from a weather station   !3839.35N/12120.19W_232/002g006t072r000P000p000h51b10138.VWS-DavisVVue   
  // This worked   String  packet[45] = "!3812.43N/12234.14W_000/000t086b10065L340F+030" ; at least on the th-d72a did not try to digi it
  //                                     012345678901234567890123456789012345678901234567890123456789
  //                                     0         1         2         3         4         5
  //  testpacket[45] = '!3833.21N/12130.44W_000/000t086L340b10065F+030' ;  cant read it into the char this way see below
  //                012345678901234567890123456789012345678901234567890123456789
  //                0         1         2         3         4         5
  // Page 65 of aprs protocol version 1.0, t=056 deg F, Lumin=320, barom=1008.5, Flood=-3.5
  // Has Petaluma coords, need to overwrite data after the wind and gust
  // Can't get Flood to work on aprs.fi anyway, will use telemetry
  //
char testpacket[47] = {'!', '3','8','3','3','.','2','1','N','/','1','2','1','3','0','.','4','4','W','_','.','.','.','/','.','.','.','t','0','8','8','b','1','0','0','7','5','L','3','4','0','F','+','0','3','0'} ;  
         //             0    1   2   3   4   5   6   7   8   9   0   1   2   3   4   5   6   7   8   9   0   1   2   3   4   5   6   7   8   9   0   1   2   3   4   5   6   7   8   9   0   1   2   3   4   5  6   7   890123456789
         //             0                                        1                                       2                                       3                                       4                           5
//       lcd.print(SYMBOLTABLEID); //Symbol Table ID from void set above  Need sim for the ! and the _
  packet[0]='!';
    for (i=1;i<19;i++){    
      packet[i]=EEPROM.read(100+i);
    }
  // packet 1-18  are as pulled in from the gps above
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
  packet[41] = 'w' ;
  packet[42] = 'R';   // need to learn if or how this works
  packet[43] = 'R' ;
  packet[44] = 'R' ;
  //
  // Assemble Telemetry Water Level Packet  *******************************************************************************************************************
  //
  // Will send a T packet "T#sss,111" where sss is a serial number and a 111 is 
  // convert n which will be 1 to 999 to a serial number, there must be a better way to do this
    if (n > 99) {
     String NNN = String(n);
     SER[0] = NNN[0];
     SER[1] = NNN[1];
     SER[2] = NNN[2];
     }
    else if (n < 10)
     {
     String NNN = String(n);
     SER[0] = '0';
     SER[1] = '0';
     SER[2] = NNN[0];
     }
     else
     {
     String NNN = String(n);
     SER[0] = '0';
     SER[1] = NNN[0];
     SER[2] = NNN[1];
     }
//
  telempkt[0]='T' ;
  telempkt[1]='#' ;
  telempkt[2]=SER[0] ;
  telempkt[3]=SER[1] ;
  telempkt[4]=SER[2] ;
  telempkt[5]=',' ;
  telempkt[6]=lineb[0] ; 
  telempkt[7]=lineb[1] ;
  telempkt[8]=lineb[2] ;
  //  not sure I need to do this beyond here, testing
  telempkt[9]=',' ;
  telempkt[10]='0' ;
  telempkt[11]='0' ;
  telempkt[12]='0' ;
  telempkt[13]=',' ;
  telempkt[14]='0' ;
  telempkt[15]='0' ;
  telempkt[16]='0' ; 
  telempkt[17]=',' ;
  telempkt[18]='0';
  telempkt[20]='0' ;
  telempkt[21]='0';
  telempkt[22]=',' ;
  telempkt[23]='0' ;
  telempkt[24]='0' ;
  telempkt[25]='0' ;
  telempkt[26]=',' ; 
  telempkt[27]='0' ;
  telempkt[28]='0' ;
    telempkt[29]='0' ;
  telempkt[30]='0' ;
  telempkt[31]='0' ;
  telempkt[32]='0' ;
  telempkt[33]='0' ;
  telempkt[34]='0' ;
  //
  // Assemble Tweet to 73S  *******************************************************************************************************************
  //
  // Say "Tide=43, Temp=56"  read '43' as 4.3'  temp in degrees F
  // "via APRS" is tacked on by 73S
  //
  // convert string lineb (dist from sonar to water) into a 'tide' of sorts.  Assume sonar
  // height is at a Tide = 10.0'
  //
  tt1 = atoi(lineb)/1000;  // no idea why the /1000  is needed, convert string to integer, say '028' to 28
  tt2 = ((120-tt1)*10/12);  // reuse tt, (120-28)/12  x 10 is 76.666, and since integer just 77
                            // and you'll read it as tide is 7.7'  
  String(tt2).toCharArray(TTT, 3);                          
     if (tt2 < 10)
      {
      TTT[2] = TTT[0];
      TTT[0] = '0';
      TTT[1] = '0';
      }
     else if (t>=10 && t<100)  
      {
      TTT[2] = TTT[1];
      TTT[1] = TTT[0];
      TTT[0] = '0';
      }
     //
  tweetpkt[0]=':';
  tweetpkt[1]='7';   // 1
  tweetpkt[2]='3';   // 2
  tweetpkt[3]='S';   // 3
  tweetpkt[4]=' ';   // 4
  tweetpkt[5]=' ';   // 5
  tweetpkt[6]=' ';   // 6
  tweetpkt[7]=' ';   // 7
  tweetpkt[8]=' ';   // 8
  tweetpkt[9]=' ';   // 9
  tweetpkt[10]=':';
  tweetpkt[11]='T';   // 1
  tweetpkt[12]='i';   // 1
  tweetpkt[13]='d';   // 2
  tweetpkt[14]='e';   // 3
  tweetpkt[15]='=';   // 4
  tweetpkt[16]=TTT[0];   // 5
  tweetpkt[17]=TTT[1];   // 6
  tweetpkt[18]=TTT[2];   // 7
  tweetpkt[19]=',';   // 8
  tweetpkt[20]=' ';   // 9
  tweetpkt[21]='T';   // 10
  tweetpkt[22]='e';   // 11
  tweetpkt[23]='m';   // 12
  tweetpkt[24]='p';   // 13
  tweetpkt[25]='=';   // 14
  tweetpkt[26]=PTMP[0];   // 15
  tweetpkt[27]=PTMP[1];   // 16
  tweetpkt[28]=PTMP[2];   // 17
    //lcd.clear();  
    //lcd.print("tt1=");
    //lcd.print(tt1);
    //lcd.print("  ");
    //lcd.print("tt2=");
    //lcd.print(tt2);
    //delay(2000);
    lcd.clear();
    // lcd.print("TTT=");
    // lcd.print(TTT[0]);
    // lcd.print(TTT[1]);
    // lcd.print(TTT[2]);
    // delay(1000);
  //
  // Radio transmit the packets
  // We want Telemetry tide data every 6 mins, and weather every, say, 5th time or 30 minutes
  // Will just use delay function to get every 6 mins
  // Use mod function to get weather sent. ...993%5=3, 994%5=4,995%5=0, send if 0  
  //
  // Send telemetry pkt every 6 mins
  //
 delay(3300); // 330,000 ms is 330 sec or 5.5 min
  lcd.clear();
  //
   if (flagTestTransmit == 1) {
    digitalWrite(13, HIGH);
    delay(500);
    //
    for (i=0;i<46;i++){       
      Serial.print(testpacket[i]);
    }
    Serial.println("");
    delay(2000);
    digitalWrite(13, LOW);
    lcd.println("Sent Test Pkt");
    delay(1000); 
// .......................................................
      lcd.clear();
      for (i=0;i<15;i++){       
      lcd.print(testpacket[i]);
      }
      lcd.setCursor(0,1);
      for (i=15;i<31;i++){       
      lcd.print(testpacket[i]);
      }
      delay(5000); 
      lcd.clear();     
      for (i=30;i<40;i++){       
      lcd.print(testpacket[i]);
      }
      lcd.setCursor(0,1);
      for (i=39;i<47;i++){       
      lcd.print(testpacket[i]);
      }
      delay(10000); 
//  ........................................................    
    } else {
    lcd.println("No Test Pkt Sent");   
    delay(1000);  
    }
   lcd.clear();
  //
 if (flagRadioTransmitTelem == 1 && n%1 == 0) {
    digitalWrite(13, HIGH);
    delay(500);
    //
    for (i=0;i<36;i++){       
      Serial.print(telempkt[i]);
    }
    Serial.println("");
    delay(2000);
    digitalWrite(13, LOW);
    lcd.println("Sent Telem Pkt");
    delay(10000);                  
    } else {
    lcd.println("No send Telem Pkt");   
    delay(1000);  
    }
   lcd.clear();
   //
   // Send Wx packet every 5th time make n%5 equal to 0, for every time make n%1 equal to 0
   //
    if (flagRadioTransmitWx == 1 && n%1 == 0) {
    digitalWrite(13, HIGH);
    delay(500);
    //
    for (i=0;i<45;i++){    // info from 0 to 44, 45 is null   
      Serial.print(packet[i]);
    }
    Serial.println("");
    delay(2000);
    digitalWrite(13, LOW);
    lcd.println("Sent Wx Pkt:");
    delay(10000);          
      lcd.clear();
      for (i=0;i<15;i++){       
      lcd.print(packet[i]);
      }
      lcd.setCursor(0,1);
      for (i=15;i<31;i++){       
      lcd.print(packet[i]);
      }
      delay(5000); 
      lcd.clear();     
      for (i=30;i<40;i++){       
      lcd.print(packet[i]);
      }
      lcd.setCursor(0,1);
      for (i=39;i<47;i++){       
      lcd.print(packet[i]);
      }
      delay(5000); 
      lcd.clear();     
  } else {
    lcd.println("No send Wx Pkt");   
    delay(1000);  
    }
   //
   // Tweet the Wx and Tide every 5th time, since Wx packet sent every
   // fifth time as well, put a 1 min gap in
   //
    delay(10000);
    if (flagRadioTransmitTweet == 1 && n%1 == 0) {
    digitalWrite(13, HIGH);
    delay(500);
    //
    for (i=0;i<30;i++){    //    
      Serial.print(tweetpkt[i]);
    }
    Serial.println("");
    delay(2000);
    digitalWrite(13, LOW);
    lcd.clear();     
    lcd.println("Sent Tweet Pkt");
    delay(10000);                   
  } else {
    lcd.println("No send Tweet Pkt");   
    delay(1000);  
    }
    //
  lcd.clear();
  if (n>=999){
    n=0;
  } 
 }


