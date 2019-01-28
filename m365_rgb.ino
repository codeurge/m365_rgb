#include "FastLED.h"
#include <EEPROM.h>

#define NUM_LEDS 14
#define FRAMES_PER_SECOND 300

CRGB leftLeds[NUM_LEDS];
CRGB rightLeds[NUM_LEDS];

int button1 = 2;
int button2 = 3;

long buttonTimer = 0;
long buttonTime = 500;
boolean buttonActive = false;
boolean longPressActive = false;
boolean button1Active = false;
boolean button2Active = false;
int buttonLastPressed = 0;

boolean turningLeft = false;
boolean turningRight = false;

boolean initEffectRan = false;
int initFrame = 0;
boolean blackOut = true;
uint8_t selectedEffect = 0;

typedef void (*SimpleEffectList[])();
SimpleEffectList effects = { confetti, sinelon, sinelonWithGlitter, strobe, rainbow, rainbowWithGlitter, juggle};

uint8_t gHue = millis(); // rotating "base color" used by many of the patterns

void setup() {
  digitalWrite(button1, INPUT_PULLUP);
  digitalWrite(button2, INPUT_PULLUP);
  FastLED.addLeds<NEOPIXEL, 5>(leftLeds, NUM_LEDS);
  FastLED.addLeds<NEOPIXEL, 6>(rightLeds, NUM_LEDS);
}

void loop() {
  FastLED.show();
  EVERY_N_MILLISECONDS( 20 ) { gHue++; } // slowly cycle the "base color" through the rainbow
  EVERY_N_MILLISECONDS(3) {

  // if left button pressed
  if (digitalRead(button1) == LOW) {

    if (buttonActive == false) {

      buttonActive = true;
      buttonTimer = millis();

    }

        button1Active = true;
        buttonLastPressed = 1;
        if (longPressActive == false) {
            for (int i =0; i < NUM_LEDS; i++) {
              leftLeds[i] = CRGB::White;
            }
        }

  }

  // if right button pressed
  if (digitalRead(button2) == LOW) {

    if (buttonActive == false) {

      buttonActive = true;
      buttonTimer = millis();

    }

    button2Active = true;
    buttonLastPressed = 2;    
    if (longPressActive == false) {
    for (int i =0; i < NUM_LEDS; i++) {
    rightLeds[i] = CRGB::White;
    }
    }

  }

  // if button is longPressed but not yet set
  if ((buttonActive == true) && (millis() - buttonTimer > buttonTime) && (longPressActive == false)) {

    longPressActive = true;
    if (button1Active == true && button2Active == false) {
      for (int i =0; i < NUM_LEDS; i++) {
        leftLeds[i] = CRGB::White;
        rightLeds[i] = CRGB::White;
      }
      if (blackOut == true) { blackOut = false; } else { prevPattern(); }

    }

    if (button2Active == true && button1Active == false) {
      for (int i =0; i < NUM_LEDS; i++) {
        leftLeds[i] = CRGB::White;
        rightLeds[i] = CRGB::White;
      }      
      if (blackOut == true) { blackOut = false; } else { nextPattern(); }
    }
    if (button1Active == true && button2Active == true) {
      for (int i =0; i < NUM_LEDS; i++) {
        leftLeds[i] = CRGB::Purple;
        rightLeds[i] = CRGB::Purple;
      }
      blackOut = !blackOut;
    }
  }

  // if button is longPressed but has been let go
  if ((buttonActive == true) && (digitalRead(button1) == HIGH) && (digitalRead(button2) == HIGH)) {

    if (longPressActive == true) {

      longPressActive = false;

    }
    
    buttonActive = false;
    button1Active = false;
    button2Active = false;

  }

  if ((buttonActive == false) && (digitalRead(button1) == HIGH) && (digitalRead(button2) == HIGH)) {
    if (blackOut == false && (millis() - buttonTimer) < buttonTime && (millis() - buttonTimer) > 0) {
      if (buttonLastPressed == 1 && ((millis() - buttonTime) > 250)) {
        turningRight = false;
        turningLeft = !turningLeft;
        buttonLastPressed = 0;
      }
      if (buttonLastPressed == 2 && ((millis() - buttonTime) > 250)) {
        turningLeft = false;
        turningRight = !turningRight;
        buttonLastPressed = 0;
      }
    }
    
    if (initEffectRan == false) {
      if (initFrame >= 148) { initEffectRan = true; }
      initEffect();
  } else {
    if (blackOut == true) { goBlack(); }
    else if (turningLeft == true) { turnLeft(); }
    else if (turningRight == true) { turnRight(); }
    else { effects[selectedEffect](); }
  }
  }  
}
}

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))
void nextPattern()
{
  // add one to the current pattern number, and wrap around at the end
  selectedEffect = (selectedEffect + 1) % ARRAY_SIZE( effects);
}

void prevPattern()
{
  // add one to the current pattern number, and wrap around at the end
  if (selectedEffect == 0) {
    selectedEffect = ARRAY_SIZE(effects) - 1;
  } else {
    selectedEffect = selectedEffect - 1;
  }
}

void sinelon()
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leftLeds, NUM_LEDS, 64);
  fadeToBlackBy( rightLeds, NUM_LEDS, 64);
  int pos = beatsin8( 50, 0, NUM_LEDS-1 );
  leftLeds[pos] += CHSV( gHue, 255, 192);
  rightLeds[pos] += CHSV( gHue, 255, 192);
}

void sinelonWithGlitter() {
  sinelon();
  addGlitter(20);
}

void rainbow() 
{
  // FastLED's built-in rainbow generator
  fill_rainbow( leftLeds, NUM_LEDS, gHue, 7);
  fill_rainbow( rightLeds, NUM_LEDS, gHue, 7);
}

void rainbowWithGlitter() 
{
  // built-in FastLED rainbow, plus some random sparkly glitter
  rainbow();
  addGlitter(80);
}

void addGlitter( fract8 chanceOfGlitter) 
{
  if( random8() < chanceOfGlitter) {
    leftLeds[ random16(NUM_LEDS) ] += CRGB::White;
    rightLeds[ random16(NUM_LEDS) ] += CRGB::White;
  }
}

void strobe() {
  fadeToBlackBy( leftLeds, NUM_LEDS, 64);
  fadeToBlackBy( rightLeds, NUM_LEDS, 64);
  addGlitter(80);
}

void confetti() 
{
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leftLeds, NUM_LEDS, 10);
  fadeToBlackBy( rightLeds, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS);
  leftLeds[pos] += CHSV( gHue + random8(64), 200, 255);
  rightLeds[pos] += CHSV( gHue + random8(64), 200, 255);
}

void juggle() {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leftLeds, NUM_LEDS, 20);
  fadeToBlackBy( rightLeds, NUM_LEDS, 20);
  byte dothue = 0;
  for( int i = 0; i < 8; i++) {
    leftLeds[beatsin16( i+7, 0, NUM_LEDS-1 )] |= CHSV(dothue, 200, 255);
    rightLeds[beatsin16( i+7, 0, NUM_LEDS-1 )] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
}

void goBlack() {
    for( int i = 0; i < NUM_LEDS; i++) {
      leftLeds[i] = CRGB::Black;
      rightLeds[i] = CRGB::Black;
      FastLED.show();
      delay(10);
    }
}

void initEffect() {
  fadeToBlackBy( leftLeds, NUM_LEDS, 16);
  fadeToBlackBy( rightLeds, NUM_LEDS, 16);
  int pos = beatsin8( 75, 0, NUM_LEDS-1 );
  leftLeds[pos] = CRGB::Red;
  rightLeds[pos] = CRGB::Red;
  initFrame = initFrame + 1;
}

void turnLeft() {
  fadeToBlackBy( leftLeds, NUM_LEDS, 16);
  int pos = beatsin8( 75, 0, NUM_LEDS-1 );
  leftLeds[pos] = CRGB::Yellow;
  rightLeds[pos] = CRGB::White;
  initFrame = initFrame + 1;
}

void turnRight() {
  fadeToBlackBy( rightLeds, NUM_LEDS, 16);
  int pos = beatsin8( 75, 0, NUM_LEDS-1 );
  rightLeds[pos] = CRGB::Yellow;
  leftLeds[pos] = CRGB::White;
  initFrame = initFrame + 1;
}
