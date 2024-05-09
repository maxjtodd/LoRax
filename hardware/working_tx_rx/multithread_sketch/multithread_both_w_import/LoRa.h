#ifndef LORA_H
#define LORA_H

#include "LoRaWan_APP.h"
#include <Wire.h>
#include "HT_SSD1306Wire.h"
#include "Arduino.h"

void SendReceivecode(void* pvParameters);
void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr);
void OnTxDone();
void OnTxTimeout();

#endif  // LORAWAN_HANDLER_H