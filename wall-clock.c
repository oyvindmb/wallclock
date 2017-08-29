/* ======================= includes ================================= */
#include <SparkTime.h>
#include "application.h"
#include <neopixel.h>

SYSTEM_MODE(AUTOMATIC);

/* ======================= extra ===================================== */

#define PIXEL_COUNT 60
#define PIXEL_PIN D2
#define PIXEL_TYPE WS2812B

Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, PIXEL_TYPE);

// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.

UDP UDPClient;
SparkTime rtc;

unsigned long currentTime;
unsigned long lastTime = 0UL;

void setup() {
  //rtc.begin(&UDPClient, "north-america.pool.ntp.org");
  rtc.begin(&UDPClient, "no.pool.ntp.org");
  rtc.setTimeZone(1); // gmt offset
  rtc.setUseEuroDSTRule(true);
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
}

void loop() {
  wallClock();
  //inverseWallClock();
}
  
void wallClock() {
  currentTime = rtc.now();
  if (currentTime != lastTime) {
    uint8_t sec = rtc.second(currentTime);  
    uint8_t min = rtc.minute(currentTime);
    uint8_t hour = rtc.hour(currentTime);
    
    // Adjust for 24 hour clock
    if (hour > 12) {
      hour = hour - 12;
    }
    
    // Convert from 12 hour clock to neo pixel number
    hour = hour*5;

    // Gradually move hour "arm"
    hour = hour + (int) min / 12;
    if (hour > 60) { hour = hour - 60; }
    
    // Set clock pixels
    int i,j;
    int high = 255;
    int low = 50;
    int med = 100;
    
    // Pixel 0 is in position minute 1
    for(i=1; i<strip.numPixels()+1; i++) {
      int arms[3] = {sec, min, hour};
      int color[3] = {0, 0, 0};
      int sub = low;
        
      for (j=0; j<3; j++) {
	if (arms[j] == 0) {
	  arms[j] = 60;
	}
	    
	// Since each hour pixel is reversed, the surrounding pixels need a bit more intensity to appear "right"
	sub = low;
	if (arms[j]%5 == 0) {
	  sub = med;
        }
        
	if (i == arms[j]) { // Arm value has full intensity
	  color[j] = high;
	} else if (i == arms[j]-1 || i == arms[j]+1) { // Surrounding arm pixels have reduced intensity 
	  color[j] = sub; 
	} else if ((arms[j] == 60 && i == 1) || (arms[j] == 1 && i == 60))  { // Accounts for crossing noon/midnight
	  color[j] = sub; 
	}
      }
      // Pixel 0 is on minute 1, the colors are blue for seconds, red for hour.
      strip.setPixelColor(i-1, strip.Color(color[2],color[1],color[0]));
    }
    strip.show();
  }
  lastTime = currentTime;
  delay(50);
}

void inverseWallClock() {
  // Does sort of the inverse of the regular clock, which is very colorful,
  // but not really very readable

  currentTime = rtc.now();
  if (currentTime != lastTime) {
    uint8_t sec = rtc.second(currentTime);  
    uint8_t min = rtc.minute(currentTime);
    uint8_t hour = rtc.hour(currentTime);
    
    // Adjust for 24 hour clock
    if (hour > 12) {
      hour = hour - 12;
    }
    
    // Convert from 12 hour clock to neo pixel number
    hour = hour*5;

    // Gradually move hour "arm"
    hour = hour + (int) min / 12;
    if (hour > 60) { hour = hour - 60; }
    
    int i,j;
    // Set the wheel color first
    for(j=0; j<256; j++) {
      for(i=0; i< strip.numPixels(); i++) {
        if (i != sec && i != sec-1 && i != sec+1 && i != min && i != min-1 && i != min+1 && i != hour-1 && i != hour && i != hour +1) {
          strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
        }
      }
    }

    // Set clock pixels
    // Pixel 0 is in position minute 1
    for(i=1; i<strip.numPixels()+1; i++) {
      int arms[3] = {sec, min, hour};
      int color[3] = {0, 0, 0};
        
      for (j=0; j<3; j++) {
	if (arms[j] == 0) {
	  arms[j] = 60;
	}
	    
	if (j == 0 && (i == arms[j])) { // Main arm for second hand
	  strip.setPixelColor(i-1, strip.Color(color[2],color[1],color[0]));
	} else if (j == 1 && (i == arms[j] || i == arms[j]-1 || i == arms[j]+1)) { // Main and two surrounding black for minute hand
	  strip.setPixelColor(i-1, strip.Color(color[2],color[1],color[0]));
	} else if (j == 2 && (i == arms[j] || i == arms[j]-1 || i == arms[j]+1 || i == arms[j]-2 || i == arms[j]+2)) { // Main and FOUR surrounding black for hour hand
	  strip.setPixelColor(i-1, strip.Color(color[2],color[1],color[0]));
	} else if ((arms[j] == 60 && i == 1) || (arms[j] == 1 && i == 60))  { // Accounts for crossing noon/midnight
	  // TODO this ok still?
          strip.setPixelColor(i-1, strip.Color(color[2],color[1],color[0]));
	}
      }
    }
    strip.show();
  }
  lastTime = currentTime;
  delay(50);
}
