// #include <BLEDevice.h>
// #include <BLEUtils.h>
// #include <BLEServer.h>
// #include "heltec.h" // Include Heltec OLED library

#include "LoRaWan_APP.h"
#include <Wire.h>             //| Included via heltec.h
//#include "HT_SSD1306Wire.h"   //|  |
#include "Arduino.h"          //| Included via loranwan

#include "LoRa.h"

TaskHandle_t Task0;
TaskHandle_t Task1;

void setup() {
  Serial.begin(115200);
  Serial.println();

  //create a task that executes the Bluetoothcode() function, with priority 1 and executed on core 0
  xTaskCreatePinnedToCore(Bluetoothcode, "Task0", 10000, NULL, 1, &Task0, 0);
  //create a task that executes the SendReceive() function, with priority 1 and executed on core 1
  xTaskCreatePinnedToCore(SendReceivecode, "Task1", 10000, NULL, 1, &Task1, 1);
}

void loop() {
  // nothing to do here, everything happens in the Task1Code and Task2Code functions
  Serial.print("main loop on core ");
  Serial.println(xPortGetCoreID());
  delay(3000);
}

void Bluetoothcode(void* pvParameters) {
  Serial.print("Task1 running on core ");
  Serial.println(xPortGetCoreID());

  for (;;) {
    Serial.println("  Core 1 processing - send/receive");
    delay(1000);

    /* INSERT CODE TO MONITOR RECEIVING & SENDING */
  }
}



