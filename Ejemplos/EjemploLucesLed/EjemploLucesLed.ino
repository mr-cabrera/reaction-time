#include <Adafruit_NeoPixel.h>

#define LED_PIN     18   // Pin donde se conecta el DIN de la tira
#define NUM_PIXELS  3    // Número de LEDs en la tira

Adafruit_NeoPixel pixels(NUM_PIXELS, LED_PIN, NEO_RGB + NEO_KHZ800);

void setup() {
  pixels.begin();       // Inicializa la tira
  pixels.clear();       // Apaga todos los LEDs al inicio
  pixels.show();
}

void loop() {

//setColor(255, 0, 0);
//delay(2000);
//setColor(0, 255, 0);
//delay(2000);
setColor(0, 0, 255);
//delay(2000);
//pixels.setPixelColor(0, pixels.Color(255, 255, 255));
//pixels.setPixelColor(1, pixels.Color(255, 255, 255));
//pixels.setPixelColor(2, pixels.Color(255, 255, 255));
/*pixels.show();
delay(3000);
pixels.setPixelColor(0, pixels.Color(0, 0, 255));
pixels.setPixelColor(1, pixels.Color(0, 0, 255));
pixels.setPixelColor(2, pixels.Color(0, 0, 255));
/*pixels.show();
delay(3000);
pixels.setPixelColor(0, pixels.Color(255, 0, 0));
pixels.setPixelColor(1, pixels.Color(255, 0, 0));
pixels.setPixelColor(2, pixels.Color(255, 0, 0));
pixels.show();
delay(3000);
pixels.setPixelColor(0, pixels.Color(0, 255, 0));
pixels.setPixelColor(1, pixels.Color(0, 255, 0));
pixels.setPixelColor(2, pixels.Color(0, 255, 0));
pixels.show();
delay(3000);*/
  // Efecto arcoíris
  //rainbowCycle(5);
}

// --- Función para efecto arcoíris ---
void rainbowCycle(int wait) {
  uint16_t i, j;

  for (j = 0; j < 256 * 5; j++) { // 5 ciclos de arcoíris
    for (i = 0; i < pixels.numPixels(); i++) {
      pixels.setPixelColor(i, Wheel(((i * 256 / pixels.numPixels()) + j) & 255));
    }
    pixels.show();
    delay(wait);
  }
}

// --- Función auxiliar para el color arcoíris ---
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    return pixels.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if (WheelPos < 170) {
    WheelPos -= 85;
    return pixels.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return pixels.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

void setColor(int r, int g, int b) {
  for (int i = 0; i < NUM_PIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(r, g, b));
  }
  pixels.show();
}
