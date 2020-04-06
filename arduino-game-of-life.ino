#include <Adafruit_NeoPixel.h>
#include <Math.h>

#define PIXEL_DATA_PIN (6)
#define NUM_LIGHTS     (64)
#define LIFE           ((unsigned long)128<<8 | 255)
#define DEATH          ((unsigned long)0)
#define TOUCHED        ((unsigned long)96<<8)

// When setting up the NeoPixel library, we tell it how many pixels,
// and which pin to use to send signals. Note that for older NeoPixel
// strips you might need to change the third parameter -- see the
// strandtest example for more information on possible values.
Adafruit_NeoPixel pixels(NUM_LIGHTS, PIXEL_DATA_PIN, NEO_GRB + NEO_KHZ800);

bool cells[NUM_LIGHTS];
bool touched[NUM_LIGHTS];

void refresh() {
  for(int i=0; i<NUM_LIGHTS; ++i) {
    writePixel(i, cells[i] ? LIFE : DEATH);
  }
}

unsigned long getColorAsRGB(unsigned long hsvColor) {
  byte result[3];
  
  float h = (float)((byte)(hsvColor>>16) & 0xFF) / 256.0;
  float s = (float)((byte)(hsvColor>>8) & 0xFF) / 256.0;
  float v = (float)((byte)(hsvColor) & 0xFF) / 256.0;

  h=h - (int)h;
  s=s - (int)s;
  v=v - (int)v;

  float r, g, b;
  float i, f, p, q, t;

  i = floor(h * 6);
  f = h * 6 - i;
  p = v * (1 - s);
  q = v * (1 - f * s);
  t = v * (1 - (1 - f) * s);

  switch ((int)i % 6) {
      case 0: r = v; g = t; b = p; break;
      case 1: r = q; g = v; b = p; break;
      case 2: r = p; g = v; b = t; break;
      case 3: r = p; g = q; b = v; break;
      case 4: r = t; g = p; b = v; break;
      case 5: r = v; g = p; b = q; break;
  }

  return (unsigned long)round(r*255)<<16 | (unsigned long)round(g*255)<<8 | (unsigned long)round(b*255);
}

void reset() {
  randomSeed((unsigned long)analogRead(0) * micros());
  
  for(int i=0; i<NUM_LIGHTS; ++i) {
    setPixel(i, (unsigned long)0);
    touched[i] = false;
  }
  writePixel(0, (unsigned long)255<<8);
  delay(500);
  writePixel(0, (unsigned long)0);

  for(int i=0; i<NUM_LIGHTS; ++i) {
    int r = (int)floor(i/8);
    int c = (int)i%8;
    setPixel(i, random(100) > 50 && (r>1 && r<6 && c>1 && c<6)? LIFE : DEATH);
  }
  refresh();

}

bool detectLife() {
  bool life = false;
  for(int i=0; i<NUM_LIGHTS; ++i) {
    life |= isCellAlive(i) ? 1 : 0;
  }
  return life;
}

void setup() {
    Serial.begin(115200);
    pixels.begin();
    pixels.clear();
    pixels.setBrightness(0x2);
    pixels.show();

    reset();
}

void setPixel(byte pixel, unsigned long color) {
  cells[pixel] = color;
}

void writePixel(byte pixel, unsigned long color) {
    byte r = (color >> 16) & 0xff;
    byte g = (color >> 8) & 0xff;
    byte b = (color) & 0xff;
    pixels.setPixelColor(pixel, pixels.Color(r, g, b));
    pixels.show();  
}

bool isCellAlive(byte pixel) {
  return cells[pixel];
}

bool isCellAlive(byte p0, int rr, int rc) {
    int r0 = (int)floor(p0 / 8);
    int c0 = p0 % 8;
    int c = c0 + rc;
    int r = r0 + rr;
    if( c < 0 ) { 
      c += 8;
    } else if( c > 7 ) {
      c -= 8;
    }
    if( r < 0 ) {
      r += 8;
    } else if( r > 7 ) {
      r -= 8;
    }

    byte p = r * 8 + c;
    return isCellAlive(p);
}

long t0 = 0;
void loop() {
    int count[NUM_LIGHTS];
    delay(200);
    for(int i=0; i<NUM_LIGHTS; ++i) {
      int n = 0;
      if( isCellAlive(i, -1, -1) ) ++n;
      if( isCellAlive(i, -1, +0) ) ++n;
      if( isCellAlive(i, -1, +1) ) ++n;
      if( isCellAlive(i, +0, -1) ) ++n;
      if( isCellAlive(i, +0, +1) ) ++n;
      if( isCellAlive(i, +1, -1) ) ++n;
      if( isCellAlive(i, +1, +0) ) ++n;
      if( isCellAlive(i, +1, +1) ) ++n;
      count[i] = n;
    }

    for(int i=0; i<NUM_LIGHTS; ++i) {
      if( isCellAlive(i) && (count[i]<2 || count[i]>3) ) {
        setPixel(i, DEATH);
      } else if( !isCellAlive(i) && (count[i]==3) ) {
        setPixel(i, LIFE);
      }
      touched[i] = touched[i] | cells[i];
    }
    refresh();
    
    if( (millis() - t0 > 15000) || !detectLife()) {
      reset();
      t0 = millis();
    }
}
