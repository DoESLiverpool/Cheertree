/*

 CheerLights!
 
 CheerLights Channel --> Arduino Ethernet --> Addressable LED strip
 
 requires: 
     HTTP Client Library
     https://github.com/amcewen/HttpClient
     FastLED library
     http://fastled.io/
*/

// Undefine this to get the Ethernet version
#define WIFI


#include <SPI.h>
#include <HttpClient.h>
#ifdef WIFI
#include <WiFi.h>
#else
#include <Ethernet.h>
#include <EthernetClient.h>
#endif
#include "FastLED.h"

// Color Effects Setup
#define NUM_LEDS 152 // Total # of lights on string (usually 50, 48, or 36)

#define WARMWHITE 0xFFDFDF
#define OLDLACE   0xFFDFDF
#define WHITE     0xFFFFFF
#define BLACK     0x000000
#define RED       0xFF0000
#define GREEN     0x00FF00
#define BLUE      0x0000FF
#define CYAN      0x00FFFF
#define MAGENTA   0xFF00FF
#define YELLOW    0xFFFF00
#define PURPLE    0x800080
#define ORANGE    0xFF5A00
#define PINK      0xCBC0FF

#define NO_STAR	-1
// Position of the LED which is the "star" on the tree
// Set to NO_STAR to not have a star LED
int starLED = 151;
// Number of colours in the starColours sequence
const int starColoursCount = 10;
// Colour sequence that the "star" LED will cycle through whenever
// the main tree colour changes
uint32_t starColours[starColoursCount] = { RED, ORANGE, YELLOW, GREEN, CYAN, BLUE, MAGENTA, PURPLE };
// Final colour that the star is set to in between changes
uint32_t starEndColour = YELLOW;

#define DATA_PIN 2
#define CLOCK_PIN 3
//Define the array of LEDs
CRGB leds[NUM_LEDS];

// Local Network Settings
#ifdef WIFI
char ssid[] = "YOUR NETWORK"; //  your network SSID (name) 
char pass[] = "NETWORK PASSWORD";    // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;            // your network key Index number (needed only for WEP)

int status = WL_IDLE_STATUS;
#else
byte mac[] = { 0xD4, 0x28, 0xB2, 0xFF, 0xFF, 0xFF }; // Must be unique on local network
#endif

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
  
  // Choose your attached LED strip, colour order set uncommented
  // e.g.
  // FastLED.addLeds<WS2801, DATA_PIN, CLOCK_PIN, RGB>(leds, NUM_LEDS);
  FastLED.addLeds<WS2801, DATA_PIN, CLOCK_PIN, GRB>(leds, NUM_LEDS);
  // FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS);

  // set all the LEDs to off
  colorWipe(BLACK, 0);
  
  // show initial status
  colorWipe(WHITE, 10);
  
#ifdef WIFI
  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present"); 
    // don't continue:
    while(true);
  } 
  
  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) { 
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:    
    status = WiFi.begin(ssid, pass);
  
    colorWipe(RED, 1);
    delay(1000);
    colorWipe(ORANGE, 1);
    delay(1000);

    // wait 10 seconds for connection:
    delay(10000);
  } 
#else
  while (Ethernet.begin(mac) != 1)
  {
    Serial.println("Error getting IP address via DHCP, trying again...");
    colorWipe(RED, 1);
    delay(1000);
    colorWipe(ORANGE, 1);
    delay(1000);
    delay(15000);
  }  
#endif
  
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
#ifdef WIFI
  WiFiClient c;
#else
  EthernetClient c;
#endif
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
                  switch (c) {
                  case 'r':
                    currentCommand = RED;
                    stopParsing = true;
                    break;
                  case 'g':
                    currentCommand = GREEN;
                    stopParsing = true;
                    break;
                  case 'c':
                    currentCommand = CYAN;
                    stopParsing = true;
                    break;
                  case 'm':
                    currentCommand = MAGENTA;
                    stopParsing = true;
                    break;
                  case 'y':
                    currentCommand = YELLOW;
                    stopParsing = true;
                    break;
                  case 'p':
                    c = http.read();
                    if (c == 'u') currentCommand = PURPLE;
                    else if (c == 'i') currentCommand = PINK;
                    stopParsing = true;
                    break;
                  case 'o':
                    c = http.read();
                    if (c == 'r') currentCommand = ORANGE;
                    else if (c == 'l') currentCommand = OLDLACE;
                    stopParsing = true;
                    break;
                  case 'w':
                    c = http.read();
                    if (c == 'h') currentCommand = WHITE;
                    else if (c == 'a') currentCommand = WARMWHITE;
                    stopParsing = true;
                    break;
                  case 'b':
                    c = http.read();
                    if (c == 'l') {
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
  int starColourIdx = 0;

  for (i=0; i < NUM_LEDS; i++) {
      leds[i] = c;
      if (starLED != NO_STAR) {
        // Show a star
        leds[starLED] = starColours[starColourIdx++];
        starColourIdx = starColourIdx % starColoursCount;
      }
      FastLED.show();
      delay(wait);
  }
  if (starLED != NO_STAR) {
    // Ensure the star ends on the right colour
    leds[starLED] = starEndColour;
    FastLED.show();
  }
}



