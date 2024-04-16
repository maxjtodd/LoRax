#include "BluetoothHandler.h"
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLE2902.h>
//#include <Heltec.h> // Assuming this is where Heltec related code resides
#include "heltec.h"

#include "shared_data.h"

#define SERVICE_UUID        "12345678-1234-1234-1234-123456789abc"
#define CHARACTERISTIC_UUID "abcdefab-cdef-1234-5678-123456789abc"

bool deviceConnected = false;
bool oldDeviceConnected = false;
char data[50];

BLECharacteristic* characteristic = NULL;

void MyServerCallbacks::onConnect(BLEServer* pServer, esp_ble_gatts_cb_param_t *param) {
    Serial.println("Device connected");
    deviceConnected = true;

    // try to to retrieve the name of the connected device
    std::string name = "Device";
    if (param->connect.remote_bda) {
        char addr_str[30];

        // print mac of new device 
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

}

void MyServerCallbacks::onDisconnect(BLEServer* pServer) {
    Serial.println("Device disconnected");

    deviceConnected = false;
    pServer->startAdvertising();
}

void Bluetoothcode(void* pvParameters) {

    Serial.print("Task0 running on core ");
    Serial.println(xPortGetCoreID());

    // initialize BLE
    BLEDevice::init("Heltec BLE Server");
    BLEServer *pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    BLEService *pService = pServer->createService(SERVICE_UUID);
    characteristic = pService->createCharacteristic(
                                             CHARACTERISTIC_UUID,
                                             BLECharacteristic::PROPERTY_READ |
                                             BLECharacteristic::PROPERTY_WRITE
                                           );
    characteristic->addDescriptor(new BLE2902()); 
    pService->start();

    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);
    pAdvertising->setMaxPreferred(0x12);
    BLEDevice::startAdvertising();
    
    Serial.println("BLE server is ready, waiting for connections...");

    for (;;) { // infinite loop
        //Serial.println("  Core 0 processing - Bluetooth");
        delay(500);

        // IF MESSAGE RECEIVED FROM iOS - to send over lora 
        // data.sensorId = 1;
        // data.value = "Value";
        
        // messageDataQueue_toBT.push(data);

        // need to check both queues for messages to be sent
        // first, check for received messages

        sprintf(data, "EXAMPLE DATA");

        // if we have a message to send, and a connected device
        if (messageDataQueue_toBT.queue.size() >= 1 && deviceConnected) {
            Serial.println("    BT - MESSAGE TO BE SENT TO iOS");

            characteristic->setValue(data);
            characteristic->notify();
            Serial.print("  Message broadcast on BLE - ");
            Serial.println(data);
        }
        // no device connected, but old device connected
        else if (!deviceConnected && oldDeviceConnected) {
            delay(500);
            pServer->startAdvertising();
            Serial.println("Restarted advertising in loop");
            oldDeviceConnected = deviceConnected;
        }
        // device connected, no old device 
        else if (deviceConnected && !oldDeviceConnected) {
            oldDeviceConnected = deviceConnected;
        }

        if (messageDataQueue_toLora.queue.size() >= 1) {
            Serial.println("    BT - MESSAGE TO BE SENT TO LORA");
        }
    }
}