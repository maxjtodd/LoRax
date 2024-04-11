#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define SERVICE_UUID            "B99CFDBD-4F69-42AA-82DA-68B92D310DEA"
#define CHARACTERISTIC_UUID  "0F146B5F-2B7E-46A0-B246-36ED1867F6E7"


BLEServer* pServer = NULL;

BLECharacteristic* characteristic = NULL;
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

void setup() {
  Serial.begin(115200);

  BLEDevice::init("LoRax Testing Device");

  pServer = BLEDevice::createServer();

  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);

  characteristic = pService->createCharacteristic(CHARACTERISTIC_UUID,
                                                   BLECharacteristic::PROPERTY_WRITE |
                                                   BLECharacteristic::PROPERTY_NOTIFY |
                                                   BLECharacteristic::PROPERTY_INDICATE
                                                  );

  characteristic->addDescriptor(new BLE2902());

  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);

  // these two values must be set to work with iPhone (no idea why)
  pAdvertising->setMinPreferred(0x06); 
  pAdvertising->setMinPreferred(0x12);
  
  BLEDevice::startAdvertising();

  Serial.println("Waiting for a client connection to notify...");

}

void loop() {
  data = "Hello, world!";
  if (deviceConnected) {
    characteristic->setValue(data);
    characteristic->notify();
    Serial.println(data);
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
