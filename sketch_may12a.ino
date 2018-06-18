#include <TinyGPS.h>
#include <SPI.h>
#include "RF24.h"
#include <SoftwareSerial.h>
// Set this radio as radio number 0 or 1 or 2
bool radioNumber = 2;
// Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 7 & 8
RF24 radio(7, 8);
SoftwareSerial uart_gps(5, 6);
TinyGPS gps;

void getgps(TinyGPS &gps);

byte addresses[][6] = {"1Node", "2Node", "3Node"};

// Used to control whether this node is sending or receiving
bool role = 0;
char enter = '\n';

void setup() {
  Serial.begin(9600);
  uart_gps.begin(9600);
  Serial.println(F("RF24/examples/GettingStarted"));
  Serial.println(F("*** PRESS 'T' to begin transmitting to the other node"));

  radio.begin();

  // Set the PA Level low to prevent power supply related issues since this is a
  // getting_started sketch, and the likelihood of close proximity of the devices. RF24_PA_MAX is default.
  radio.setPALevel(RF24_PA_LOW);

  // Open a writing and reading pipe on each radio, with opposite addresses
  if (radioNumber == 1) {
    radio.openWritingPipe((uint64_t)addresses[1]);
    radio.openReadingPipe(1, (uint64_t)addresses[0]);
    radio.openReadingPipe(2, (uint64_t)addresses[2]);
  }
  else if (radioNumber == 2) {
    radio.openWritingPipe((uint64_t)addresses[2]);
    radio.openReadingPipe(1, (uint64_t)addresses[0]);
    radio.openReadingPipe(2, (uint64_t)addresses[1]);
  }
  else
  {
    radio.openWritingPipe((uint64_t)addresses[0]);
    radio.openReadingPipe(1, (uint64_t)addresses[1]);
    radio.openReadingPipe(2, (uint64_t)addresses[2]);
  }

  // Start the radio listening for data
  radio.startListening();
}

void loop() {

  // Ping Out Role
  if (role == 1) {
    radio.stopListening();
    while (uart_gps.available())
    {
      int GPSRX = uart_gps.read();
      Serial.write(GPSRX);
      if (gps.encode(GPSRX))
      {
        char i = 'c';
        radio.write( &i, sizeof(char) );
        Serial.println(F("Now sending"));
        getgps(gps);
      }
    }
    radio.startListening();
  }


  // Pong Back Role
  if ( role == 0 ) {

    if ( radio.available()) {
      // While there is data ready
      char i;
      radio.read( &i, sizeof(char) );
      if (i == 'c' or i == 'i')
      {
        float latitude, longitude;
        int year;
        byte month, day, hour, minute, second, hundredths;
        while (!radio.available()) {
        }
        radio.read( &latitude, sizeof(float) );
        while (!radio.available()) {
        }
        radio.read( &longitude, sizeof(float) );
        while (!radio.available()) {
        }
        radio.read( &year, sizeof(int) );
        while (!radio.available()) {
        }
        radio.read( &month, sizeof(byte) );
        while (!radio.available()) {
        }
        radio.read( &day, sizeof(byte) );
        while (!radio.available()) {
        }
        radio.read( &hour, sizeof(byte) );
        while (!radio.available()) {
        }
        radio.read( &minute, sizeof(byte) );
        while (!radio.available()) {
        }
        radio.read( &second, sizeof(byte) );
        while (!radio.available()) {
        }
        radio.read( &hundredths, sizeof(byte) );

        Serial.println(i);
        Serial.print("Lat/Long: ");
        Serial.print(latitude, 5);
        Serial.print(", ");
        Serial.println(longitude, 5);


        Serial.print("Date: "); Serial.print(month, DEC); Serial.print("/");
        Serial.print(day, DEC); Serial.print("/"); Serial.print(year);
        Serial.print("  Time: "); Serial.print(hour, DEC); Serial.print(":");
        Serial.print(minute, DEC); Serial.print(":"); Serial.print(second, DEC);
        Serial.print("."); Serial.println(hundredths, DEC);

        radio.stopListening();

        delay(300);

        if (latitude != 0.00000 || longitude != 0.00000)
        {

          i = 'i';
          radio.write( &i, sizeof(char) );
          radio.write( &latitude, sizeof(float) );
          radio.write( &longitude, sizeof(float) );
          radio.write( &year, sizeof(int) );
          radio.write( &month, sizeof(byte) );
          radio.write( &day, sizeof(byte) );
          radio.write( &hour, sizeof(byte) );
          radio.write( &minute, sizeof(byte) );
          radio.write( &second, sizeof(byte) );
          radio.write( &hundredths, sizeof(byte) );
        }

        radio.startListening();
      }
    }
  }

  // Change Roles via Serial Commands
  if ( Serial.available() ) {
    char c = toupper(Serial.read());
    if ( c == 'T' && role == 0 ) {
      Serial.println(F("*** CHANGING TO TRANSMIT ROLE -- PRESS 'R' TO SWITCH BACK"));
      // Become the primary transmitter (ping out)
      role = 1;

    } else {
      if ( c == 'R' && role == 1 ) {
        Serial.println(F("*** CHANGING TO RECEIVE ROLE -- PRESS 'T' TO SWITCH BACK"));
        // Become the primary receiver (pong back)
        role = 0;
        radio.startListening();
      }
    }
  }
}
void getgps(TinyGPS &gps)
{
  float latitude, longitude;

  gps.f_get_position(&latitude, &longitude);

  Serial.print("Lat/Long: ");
  Serial.print(latitude, 5);
  Serial.print(", ");
  Serial.println(longitude, 5);

  // Same goes for date and time
  int year;
  byte month, day, hour, minute, second, hundredths;
  gps.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths);
  // Print data and time
  Serial.print("Date: "); Serial.print(month, DEC); Serial.print("/");
  Serial.print(day, DEC); Serial.print("/"); Serial.print(year);
  Serial.print("  Time: "); Serial.print(hour, DEC); Serial.print(":");
  Serial.print(minute, DEC); Serial.print(":"); Serial.print(second, DEC);
  Serial.print("."); Serial.println(hundredths, DEC);
  //Since month, day, hour, minute, second, and hundr

  Serial.print("Altitude (meters): "); Serial.println(gps.f_altitude());
  // Same goes for course
  Serial.print("Course (degrees): "); Serial.println(gps.f_course());
  // And same goes for speed
  Serial.print("Speed(kmph): "); Serial.println(gps.f_speed_kmph());
  Serial.println();

  radio.write( &latitude, sizeof(float) );
  radio.write( &longitude, sizeof(float) );
  radio.write( &year, sizeof(int) );
  radio.write( &month, sizeof(byte) );
  radio.write( &day, sizeof(byte) );
  radio.write( &hour, sizeof(byte) );
  radio.write( &minute, sizeof(byte) );
  radio.write( &second, sizeof(byte) );
  radio.write( &hundredths, sizeof(byte) );
  unsigned long chars;
  unsigned short sentences, failed_checksum;
  gps.stats(&chars, &sentences, &failed_checksum);
  delay(1000);
}



