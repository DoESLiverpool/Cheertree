/*

 CheerLights!
 
 CheerLights Channel --> Arduino Ethernet --> WS2801 LED strip
 
 requires: 
     https://github.com/adafruit/Adafruit-WS2801-Library
     https://github.com/amcewen/HttpClient
 
*/

#include <SPI.h>
#include <HttpClient.h>
#include <Ethernet.h>
#include <EthernetClient.h>
#include "Adafruit_WS2801.h"

// Color Effects Setup
#define LIGHT_COUNT 152.// Total # of lights on string (usually 50, 48, or 36)

#define WARMWHITE 0xFFDFDF
#define WHITE     0xFFFFFF
#define BLACK     0x000000
#define RED       0x0000FF
#define GREEN     0x00FF00
#define BLUE      0xFF0000
#define CYAN      0xFFFF00
#define MAGENTA   0xFF00FF
#define YELLOW    0x00FFFF
#define PURPLE    0x800080
#define ORANGE    0x005AFF

uint8_t dataPin  = 2;    
uint8_t clockPin = 3;    
Adafruit_WS2801 strip = Adafruit_WS2801(LIGHT_COUNT, dataPin, clockPin);


static uint16_t c;

// Local Network Settings
byte mac[] = { 0xD4, 0x28, 0xB2, 0xFF, 0xFF, 0xFF }; // Must be unique on local network

// Name of the server we want to connect to
const char kHostname[] = "api.thingspeak.com";
// Path to download (this is the bit after the hostname in the URL
const char kPath[] = "/channels/1417/field/1/last.txt";

// We're ignoring the minimum time interval because the minimum time from the 
// http request and return is slow enough

// Number of milliseconds to wait without receiving any data before we give up
const int kNetworkTimeout = 30*1000;
// Number of milliseconds to wait if no data is available before trying again
const int kNetworkDelay = 1000;

// Variable Setup
uint32_t lastCommand = BLACK;
uint32_t currentCommand = GREEN;

void setup() {
  // Setup Serial
  Serial.begin(9600);
  delay(100);
  
  Serial.flush();
  delay(100);

  strip.begin();
  // Update LED contents, to start they are all 'off'
  strip.show();
  
  // show initial status
  colorWipe(WHITE, 2);
  
    while (Ethernet.begin(mac) != 1)
  {
    Serial.println("Error getting IP address via DHCP, trying again...");
    colorWipe(RED, 1);
    delay(1000);
    colorWipe(ORANGE, 1);
    delay(1000);
  }
  
  //show ethernet is connected
  colorWipe(lastCommand, 2);
}

void loop() {
  // change the colour has changed
  if (currentCommand != lastCommand) {
    colorWipe(currentCommand, 50);
    lastCommand = currentCommand;
  }

  int err =0;
  EthernetClient c;
  HttpClient http(c);
  
  err = http.get(kHostname, kPath);
  if (err == 0)
  {
    Serial.println("startedRequest ok");
    boolean stopParsing = false;

    err = http.responseStatusCode();
    if (err >= 0)
    {
      Serial.print("Got status code: ");
      Serial.println(err);

      // Usually you'd check that the response code is 200 or a
      // similar "success" code (200-299) before carrying on,
      // but we'll print out whatever response we get

      err = http.skipResponseHeaders();
      if (err >= 0)
      {
        int bodyLen = http.contentLength();
        Serial.println("Body returned:");
      
        // Now we've got to the body, so we can print it out
        unsigned long timeoutStart = millis();
        char c;
        // Whilst we haven't timed out & haven't reached the end of the body
        while ( (http.connected() || http.available()) &&
               ((millis() - timeoutStart) < kNetworkTimeout) )
        {
            if (http.available())
            {
                c = http.read();
                // Print out this character
                Serial.print(c);
                
                if (stopParsing == false) {
                  if (c == 'r') currentCommand = RED;
                  else if (c == 'g') {
                    currentCommand = GREEN;
                    stopParsing = true;
                  }
                  else if (c == 'c') {
                    currentCommand = CYAN;
                    stopParsing = true;
                  }
                  else if (c == 'm') {
                    currentCommand = MAGENTA;
                    stopParsing = true;
                  }
                  else if (c == 'y') {
                    currentCommand = YELLOW;
                    stopParsing = true;
                  }
                  else if (c == 'p') {
                    currentCommand = PURPLE;
                    stopParsing = true;
                  }
                  else if (c == 'o') {
                    currentCommand = ORANGE;
                    stopParsing = true;
                  }
                  else if (c == 'w') {
                    c = http.read();
                    if (c == 'h') currentCommand = WHITE;
                    else if (c == 'a') currentCommand = WARMWHITE;
                    stopParsing = true;
                  }
                  else if (c == 'b') {
                    c = http.read();
                    if (c = 'a') {
                      c = http.read();
                      if (c == 'a') currentCommand = BLACK;
                      else if (c == 'u') currentCommand = BLUE;
                    stopParsing = true;
                    }
                  }
                }
               
                bodyLen--;
                // We read something, reset the timeout counter
                timeoutStart = millis();
            }
            else
            {
                // We haven't got any data, so let's pause to allow some to
                // arrive
                delay(kNetworkDelay);
            }
        }
      }
      else
      {
        Serial.print("Failed to skip response headers: ");
        Serial.println(err);
      }
    }
    else
    {    
      Serial.print("Getting response failed: ");
      Serial.println(err);
    }
  }
  else
  {
    Serial.print("Connect failed: ");
    Serial.println(err);
  }
  http.stop();
}

// fill the dots one after the other with said color
void colorWipe(uint32_t c, uint8_t wait) {
  int i;
  
  for (i=0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, c);
      strip.show();
      delay(wait);
  }
}



