#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define SERVICE_UUID            "B99CFDBD-4F69-42AA-82DA-68B92D310DEA"
#define CHARACTERISTIC_UUID_TX  "0F146B5F-2B7E-46A0-B246-36ED1867F6E7"
#define CHARACTERISTIC_UUID_RX  "B54E3121-477C-4A86-9FE7-19292CFA415B"


BLEServer* pServer = NULL;

BLECharacteristic* txCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
String data;

class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
  }

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
  }

};

class MyClientCallbacks: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* characteristic) override{
    Serial.println("Got to onWrite in client callback");
    String rxValue = characteristic->getValue();
    if (rxValue.length() > 0) {
      Serial.println("Receiving...");
      Serial.print("Received value: ");
      Serial.println(rxValue.c_str());
    }
  }
};

void setup() {
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

}

void loop() {
  data = "Hello, world!";
  if (deviceConnected) {
    txCharacteristic->setValue(data);
    txCharacteristic->notify();
    //Serial.println(data);
    delay(1000);
  }
  if (!deviceConnected && oldDeviceConnected) {
    delay(500);
    pServer->startAdvertising();
    Serial.println("Restarted advertising in loop");
    oldDeviceConnected = deviceConnected;
  }

  if (deviceConnected && !oldDeviceConnected) {
    oldDeviceConnected = deviceConnected;
  }

  delay(500);
}

