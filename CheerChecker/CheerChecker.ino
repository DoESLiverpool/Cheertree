/*

 CheerLights!

 CheerLights Channel --> Arduino Ethernet --> Serial Output

 requires:
     HTTP Client Library
     https://github.com/amcewen/HttpClient
*/

// Undefine this to get the Ethernet version
// #define WIFI


#include <SPI.h>
#include <HttpClient.h>
#ifdef WIFI
#include <WiFi.h>
#else
#include <Ethernet.h>
#include <EthernetClient.h>
#endif
#include <SoftwareSerial.h>

#define WARMWHITE 1
#define OLDLACE   1
#define WHITE     2
#define BLACK     3
#define RED       4
#define GREEN     5
#define BLUE      6
#define CYAN      7
#define MAGENTA   8
#define YELLOW    9
#define PURPLE    10
#define ORANGE    11
#define PINK      12

uint8_t lastCommand = 0;

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

// Setup the SoftwareSerial object for sending to the StrandController
SoftwareSerial mySerial (2, 3);

void setup() {
  // Setup Serial
  Serial.begin(9600);
  delay(100);

  Serial.flush();
  delay(100);

  // start the outgoing serial port
  mySerial.begin(9600);

  // set all the LEDs to off
  mySerial.print("black");
  delay(2000);

  // show initial status
  mySerial.print("white");
  delay(2000);

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

    mySerial.print("red");
    delay(1000);
    mySerial.print("orange");
    delay(1000);

    // wait 10 seconds for connection:
    delay(10000);
  }
#else
  while (Ethernet.begin(mac) != 1)
  {
    Serial.println("Error getting IP address via DHCP, trying again...");
    mySerial.print("red");
    delay(1000);
    mySerial.print("orange");
    delay(1000);
    delay(15000);
  }
#endif

  //show ethernet is connected
  mySerial.print("green");
}

void loop() {
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
                    if (lastCommand != RED) {
                      mySerial.print("red");
                      lastCommand = RED;
                    }
                    stopParsing = true;
                    break;
                  case 'g':
                    if (lastCommand != GREEN) {
                      mySerial.print("green");
                      lastCommand = GREEN;
                    }
                    stopParsing = true;
                    break;
                  case 'c':
                    if (lastCommand != CYAN) {
                      mySerial.print("cyan");
                      lastCommand = CYAN;
                    }
                    stopParsing = true;
                    break;
                  case 'm':
                    if (lastCommand != MAGENTA) {
                      mySerial.print("magenta");
                      lastCommand = MAGENTA;
                    }
                    stopParsing = true;
                    break;
                  case 'y':
                    if (lastCommand != YELLOW) {
                      mySerial.print("yellow");
                      lastCommand = YELLOW;
                    }
                    stopParsing = true;
                    break;
                  case 'p':
                    c = http.read();
                    if (c == 'u') {
                      if (lastCommand != PURPLE) {
                        mySerial.print("purple");
                        lastCommand = PURPLE;
                      }
                    } else if (c == 'i') {
                      if (lastCommand != PINK) {
                        mySerial.print("pink");
                        lastCommand = PINK;
                      }
                    }
                    stopParsing = true;
                    break;
                  case 'o':
                    c = http.read();
                    if (c == 'r') {
                      if (lastCommand != ORANGE) {
                        mySerial.print("orange");
                        lastCommand = ORANGE;
                      }
                    } else if (c == 'l') {
                      if (lastCommand != OLDLACE) {
                        mySerial.print("oldlace");
                        lastCommand = OLDLACE;
                      }
                    }
                    stopParsing = true;
                    break;
                  case 'w':
                    c = http.read();
                    if (c == 'h') {
                      if (lastCommand != WHITE) {
                        mySerial.print("white");
                        lastCommand = WHITE;
                      }
                    } else if (c == 'a') {
                      if (lastCommand != WARMWHITE) {
                        mySerial.print("warmwhite");
                        lastCommand = WARMWHITE;
                      }
                    }
                    stopParsing = true;
                    break;
                  case 'b':
                    c = http.read();
                    if (c == 'l') {
                      c = http.read();
                      if (c == 'a') {
                        if (lastCommand != BLACK) {
                          mySerial.print("black");
                          lastCommand = BLACK;
                        }
                      } else if (c == 'u') {
                        if (lastCommand != BLUE) {
                          mySerial.print("blue");
                          lastCommand = BLUE;
                        }
                      }
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
