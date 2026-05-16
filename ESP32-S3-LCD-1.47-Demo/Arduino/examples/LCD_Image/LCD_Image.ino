/**
 ******************************************************************************
 * @file     LCD_Image.ino
 * @author   Yongqin Ou
 * @version  V1.0
 * @date     2024-10-31
 * @brief    Setup experiment for SD card, display, and RGB lamp
 * @license  MIT
 * @copyright Copyright (c) 2024, Waveshare
 ******************************************************************************
 * 
 * Experiment Objective: Learn how to set up and use SD card for image display and control an RGB lamp.
 *
 * Hardware Resources and Pin Assignment: 
 * 1. SD Card Interface --> As configured in SD_Card.h.
 * 2. Display Interface --> As configured in Display_ST7789.h.
 * 3. RGB Lamp Interface --> As configured in RGB_lamp.h.
 *
 * Experiment Phenomenon:
 * 1. Runs tests and initializes SD card, display, and sets backlight.
 * 2. Continuously loops through images on the SD card and controls the RGB lamp.
 * 
 * Notes:
 * None
 * 
 ******************************************************************************
 * 
 * Development Platform: ESP32
 * Support Forum: service.waveshare.com
 * Company Website: www.waveshare.com
 *
 ******************************************************************************
 */

/*
   You must copy the PNG File from the SD Card File folder to the SD card and connect the SD card to the device
*/
#include "SD_Card.h"
#include "Display_ST7789.h"
#include "LCD_Image.h"
#include "RGB_lamp.h"

void setup()
{
  Flash_test();
  SD_Init();     
  LCD_Init();
  Set_Backlight(90);  
}

void loop()
{
  Image_Next_Loop("/",".png",300);
  RGB_Lamp_Loop(2);
  delay(5);
}