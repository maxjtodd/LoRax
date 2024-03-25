#include "BluetoothHandler.h"
#include <BLEDevice.h>
#include <BLEUtils.h>
//#include <Heltec.h> // Assuming this is where Heltec related code resides
#include "heltec.h"

#define SERVICE_UUID        "12345678-1234-1234-1234-123456789abc"
#define CHARACTERISTIC_UUID "abcdefab-cdef-1234-5678-123456789abc"

void MyServerCallbacks::onConnect(BLEServer* pServer, esp_ble_gatts_cb_param_t *param) {
    Serial.println("Device connected");
    //Heltec.display->clear();
    // try to to retrieve the name of the connected device
    std::string name = "Device";
    if (param->connect.remote_bda) {
        char addr_str[30];
        sprintf(addr_str, "%02x:%02x:%02x:%02x:%02x:%02x",
                param->connect.remote_bda[0], param->connect.remote_bda[1], param->connect.remote_bda[2],
                param->connect.remote_bda[3], param->connect.remote_bda[4], param->connect.remote_bda[5]);
        name += " (";
        name += addr_str;
        name += ")";
    }

    Serial.print("Connected to: ");
    Serial.printf(name.c_str());
    Serial.println();
    
    // Heltec.display->drawString(0, 0, "Connected to:");
    // Heltec.display->drawString(0, 10, name.c_str());
    // Heltec.display->display();
}

void MyServerCallbacks::onDisconnect(BLEServer* pServer) {
    Serial.println("Device disconnected");
    // Heltec.display->clear();
    // Heltec.display->drawString(0, 0, "Device disconnected");
    // Heltec.display->display();
    // restart advertising after disconnection
    pServer->startAdvertising();
}

void Bluetoothcode(void* pvParameters) {
    Serial.print("Task0 running on core ");
    Serial.println(xPortGetCoreID());

    // initialize OLED display
    // Heltec.begin(true /*DisplayEnable Enable*/, false /*LoRa Disable*/, true /*Serial Enable*/);
    // Heltec.display->clear();
    // Heltec.display->drawString(0, 0, "BLE Server Start");
    // Heltec.display->display();

    // initialize BLE
    BLEDevice::init("Heltec BLE Server");
    BLEServer *pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    BLEService *pService = pServer->createService(SERVICE_UUID);
    BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                             CHARACTERISTIC_UUID,
                                             BLECharacteristic::PROPERTY_READ |
                                             BLECharacteristic::PROPERTY_WRITE
                                           );
    pCharacteristic->setValue("Hello BLE");
    pService->start();
    pServer->getAdvertising()->start();
    
    Serial.println("BLE server is ready, waiting for connections...");

    for (;;) { // infinite loop
        Serial.println("  Core 0 processing - Bluetooth");
        delay(1000);
    }
}