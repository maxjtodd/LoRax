

#include "LoRaWan_APP.h"
#include <Wire.h>             //| Included via heltec.h
//#include "HT_SSD1306Wire.h" //|  |
#include "Arduino.h"          //| Included via loranwan

#include "BluetoothHandler.h"
#include "LoRa.h"
#include "shared_data.h"


TaskHandle_t Task0;
TaskHandle_t Task1;

void setup() {


  Serial.begin(115200);

  //create a task that executes the Bluetoothcode() function, with priority 1 and executed on core 0
  xTaskCreatePinnedToCore(Bluetoothcode, "Task0", 10000, NULL, 1, &Task0, 0);
  //create a task that executes the SendReceive() function, with priority 1 and executed on core 1
  xTaskCreatePinnedToCore(SendReceivecode, "Task1", 10000, NULL, 1, &Task1, 1);
}

void loop() {
  // nothing to do here, everything happens in the Task1Code and Task2Code functions
  //Serial.print("main loop on core ");

  //Serial.printf("\nCHIP MAC: %012llx\n", ESP.getEfuseMac());
  //Serial.printf("\nCHIP MAC: %012llx\n", ESP.getChipId());

  //Serial.println(xPortGetCoreID());
  delay(5500);
}


