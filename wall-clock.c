/* ======================= includes ================================= */
#include "SparkTime/SparkTime.h"
#include "application.h"
#include "neopixel/neopixel.h"

SYSTEM_MODE(AUTOMATIC);

/* ======================= prototypes =============================== */

void rainbowCycle(uint8_t wait);
uint32_t Wheel(byte WheelPos);

/* ======================= extra ===================================== */

#define PIXEL_COUNT 60
#define PIXEL_PIN D2
#define PIXEL_TYPE WS2812B

Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, PIXEL_TYPE);

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
  //rainbowCycle(2000);
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
}

void inverseWallClock() {
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
      int color[3] = {255, 255, 255};

      for (j=0; j<3; j++) {
	if (arms[j] == 0) {
	  arms[j] = 60;
	}

	// Since each hour pixel is reversed, the surrounding pixels need a bit more intensity to appear "right"
	if (i == arms[j]) { // Arm value has full intensity
	  color[j] = 0;
	} else if (i == arms[j]-1 || i == arms[j]+1) { // Surrounding arm pixels have reduced intensity
	  color[j] = 0;
	} else if ((arms[j] == 60 && i == 1) || (arms[j] == 1 && i == 60))  { // Accounts for crossing noon/midnight
	  color[j] = 0;
	}
	if (color[0] == 0 && color[1] == 0 && color[2] == 0) {
	  // Pixel 0 is on minute 1, the colors are blue for seconds, red for hour.
	  strip.setPixelColor(i-1, strip.Color(color[2],color[1],color[0]));
	}
      }
    }
    strip.show();
  }
  lastTime = currentTime;
}

// Set all pixels in the strip to a solid color, then wait (ms)
void colorAll(uint32_t c, uint8_t wait) {
  uint16_t i;

  for(i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
  }
  strip.show();
  delay(wait);
}

// Fill the dots one after the other with a color, wait (ms) after each one
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Slightly different, this makes the rainbow equally distributed throughout, then wait (ms)
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) { // 1 cycle of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  if(WheelPos < 85) {
    return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
    WheelPos -= 170;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}
