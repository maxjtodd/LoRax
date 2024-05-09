#include "BluetoothHandler.h"
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLE2902.h>
//#include <Heltec.h> // Assuming this is where Heltec related code resides
#include "heltec.h"
#include <esp_system.h>
#include <Arduino.h>
#include "shared_data.h"
#include "esp_mac.h"


#define SERVICE_UUID            "B99CFDBD-4F69-42AA-82DA-68B92D310DEA"
#define CHARACTERISTIC_UUID_TX  "0F146B5F-2B7E-46A0-B246-36ED1867F6E7"
#define CHARACTERISTIC_UUID_RX  "B54E3121-477C-4A86-9FE7-19292CFA415B"

bool deviceConnected = false;
bool oldDeviceConnected = false;
char data[50];

// mac addr of esp32
uint8_t baseMac[6];


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

class MyClientCallbacks: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* characteristic) override{
    String rxValue = characteristic->getValue();
    char* rxString = new char[rxValue.length() + 1];
    strcpy(rxString, rxValue.c_str());
    if (rxValue.length() > 0) {
      messageData newMessage = {1, "MAC1", "MAC2", 0, rxString};
      pushMessageData(messageDataQueue_toLora, newMessage);
    }

    delete[] rxString;
  }
};

esp_err_t getMacAddress() {
    esp_err_t err = esp_read_mac(baseMac, ESP_MAC_BT);
    if (err != ESP_OK) {
        printf("Failed to read MAC address. Error code: %d\n", err);
        return err;
    }

    printf("MAC Address: %02x:%02x:%02x:%02x:%02x:%02x\n",
           baseMac[0], baseMac[1], baseMac[2],
           baseMac[3], baseMac[4], baseMac[5]);

    return ESP_OK;
}

char* buildStringFromPacket(messageData& newMessage) {

    char* result;
    sprintf(result, "%d;%s;%s;%d;%s", newMessage.message_type, newMessage.message_id, newMessage.message_dest, newMessage.size, newMessage.value);
    return result;
}

bool send = true;

void Bluetoothcode(void* pvParameters) {

  Serial.print("Task0 running on core ");
  Serial.println(xPortGetCoreID());

  // initialize BLE
  Serial.begin(115200);
  
  BLEDevice::init("LoRax");

  pServer = BLEDevice::createServer();

  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);

  txCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_TX,
                                                   BLECharacteristic::PROPERTY_WRITE |
                                                   BLECharacteristic::PROPERTY_NOTIFY |
                                                   BLECharacteristic::PROPERTY_INDICATE
                                                  );

  txCharacteristic->addDescriptor(new BLE2902());

  BLECharacteristic* rxCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_RX,
                                                   BLECharacteristic::PROPERTY_READ |
                                                   BLECharacteristic::PROPERTY_WRITE |
                                                   BLECharacteristic::PROPERTY_NOTIFY |
                                                   BLECharacteristic::PROPERTY_INDICATE
                                                  );


  rxCharacteristic->setCallbacks(new MyClientCallbacks());

  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);

  // these two values must be set to work with iPhone (no idea why)
  pAdvertising->setMinPreferred(0x06); 
  pAdvertising->setMinPreferred(0x12);
  
  BLEDevice::startAdvertising();

  Serial.println("Waiting for a client connection to notify...");

    getMacAddress();

    for (;;) { // infinite loop

        //Serial.println("  Core 0 processing - Bluetooth");
        delay(10000);

            Serial.println("Building test");

            messageData newMessage = {1, "MAC1", "MAC2", 0, "this is a test message"};
            if (send) {pushMessageData(messageDataQueue_toLora, newMessage);}
            //send = false;

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

            // get new message to be sent 
            messageData newMessage = getMessageData(messageDataQueue_toBT);

            // unpack to string
            char* toIOS = buildStringFromPacket(newMessage);

            // set characteristic to new message    
            characteristic->setValue(toIOS);
            characteristic->notify();
            Serial.print("  Message broadcast on BLE - ");
            Serial.println(toIOS);
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


        // check for message to send to LoRa core
        // 1. build packet
        //      
        // 2. add packet to messageDataQueue_toLora
    }
}