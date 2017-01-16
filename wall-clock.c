#include "SparkTime/SparkTime.h"
#include "application.h"
#include "neopixel/neopixel.h"

SYSTEM_MODE(AUTOMATIC);

void rainbowCycle(uint8_t wait);
uint32_t Wheel(byte WheelPos);

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
  wallClock();
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
