#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define PIN 9
#define MIN_INTERVAL 30
#define MAX_INTERVAL 130

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(150, PIN, NEO_GRB + NEO_KHZ800);

/* Extremely easy serial protocol that consists of 5 bytes:
 *
 * 0 - Mode (aka "what pattern")
 * 1 - red
 * 2 - green
 * 3 - blue
 * 4 - interval (used for delay())
 *
 */
byte mode_color[5] = {255, 0, 0, 0, 60};

unsigned long time = millis();

void setup() {
  strip.begin();
  strip.setBrightness(90);
  strip.show(); // Initialize all pixels to 'off'
  Serial.begin(9600);
}

void loop() {
  if(Serial.available() >= 5){
    Serial.readBytes(mode_color, 5);
    time = millis();
  }

  if(mode_color[4] < MIN_INTERVAL)
    mode_color[4] = MIN_INTERVAL;

  if(mode_color[4] > MAX_INTERVAL)
    mode_color[4] = MAX_INTERVAL;

  // If we have 30 seconds of the same, do random stuff
  if(millis() - time > 30000)
    mode_color[0] = 255;

  switch(mode_color[0]){
    case 0: 
      colorWipe(strip.Color(mode_color[1], mode_color[2], mode_color[3]), mode_color[4]);
      break;
    case 1:
      theaterChase(strip.Color(mode_color[1], mode_color[2], mode_color[3]), mode_color[4]);
      break;
    case 2:
      rainbow(mode_color[4]);
      break;
    case 3:
      rainbowCycle(mode_color[4]);
      break;
    case 4:
      theaterChaseRainbow(mode_color[4]);
      break;
    case 5:
      knightrider(strip.Color(mode_color[1], mode_color[2], mode_color[3]), 1);
    case 255:
      delay(100);
      //randomLights();
    default:
      // assume malice
      theaterChase(strip.Color(1,1,1), 10);
      break;
  }
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
  uint32_t c2 = strip.Color(0,0,0);
  for(uint16_t i=strip.numPixels(); i!=0; i--) {
    strip.setPixelColor(i, c2);
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

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

//Theatre-style crawling lights.
void theaterChase(uint32_t c, uint8_t wait) {
  for (int j=0; j<10; j++) {  //do 10 cycles of chasing
    for (int q=0; q < 3; q++) {
      for (int i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, c);    //turn every third pixel on
      }
      strip.show();

      delay(wait);

      for (int i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait) {
  for (int j=0; j < 256; j++) {     // cycle all 256 colors in the wheel
    for (int q=0; q < 3; q++) {
      for (int i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, Wheel( (i+j) % 255));    //turn every third pixel on
      }
      strip.show();

      delay(wait);

      for (int i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

// taken from Dennis de Greef
// Modified to take a color
void knightrider(uint32_t color, uint8_t wait) {
  uint8_t i;
  
  for(i = 0; i < strip.numPixels(); i++) {
    if(i == 0) {
       strip.setPixelColor(strip.numPixels() - 1, 0, 0, 0); 
    } else {
       strip.setPixelColor(i - 1, 0, 0, 0);
    }
     strip.setPixelColor(i, color);
     strip.show();
     delay(wait);
  }
  
  for(i = strip.numPixels(); i > 0; i--) {
    if(i == 0) {
       strip.setPixelColor(strip.numPixels() + 1, 0, 0, 0); 
    } else {
       strip.setPixelColor(i + 1, 0, 0, 0);
    }
     strip.setPixelColor(i, color);
     strip.show();
     delay(wait);
   }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}
