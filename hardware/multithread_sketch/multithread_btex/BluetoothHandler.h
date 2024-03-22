#ifndef BLUETOOTH_HANDLER_H
#define BLUETOOTH_HANDLER_H

#include <BLEServer.h>

class MyServerCallbacks: public BLEServerCallbacks {
    public: 
        void onConnect(BLEServer* pServer, esp_ble_gatts_cb_param_t * params);
        void onDisconnect(BLEServer* pServer);
};

void Bluetoothcode(void * pvParameters);

#endif 