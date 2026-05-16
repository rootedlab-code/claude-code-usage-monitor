#pragma once
#include "Arduino.h"

// Il core ESP32 definisce già PIN_NEOPIXEL=48 per il DevKitM-1; sulla
// Waveshare LCD-1.47 il LED RGB onboard è invece su GPIO 38.
#define RGB_LED_PIN 38

void Set_Color(uint8_t Red,uint8_t Green,uint8_t Blue);                 // Set RGB bead color
void RGB_Lamp_Loop(uint16_t Waiting);                                   // The lamp beads change color in cycles