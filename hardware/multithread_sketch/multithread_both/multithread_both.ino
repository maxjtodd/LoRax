

#include "LoRaWan_APP.h"
#include <Wire.h>             //| Included via heltec.h
#include "HT_SSD1306Wire.h"   //|  |
#include "Arduino.h"          //| Included via loranwan

#include "C:\Users\brock\OneDrive\Desktop\CS448\LoRax_repo\hardware\multithread_sketch\multithread_btex\BluetoothHandler.h"
#include "C:\Users\brock\OneDrive\Desktop\CS448\LoRax_repo\hardware\multithread_sketch\multithread_btex\BluetoothHandler.cpp"


#define RF_FREQUENCY                                915000000 // Hz
#define TX_OUTPUT_POWER                             14        // dBm
#define LORA_BANDWIDTH                              0         // [0: 125 kHz,
                                                              //  1: 250 kHz,
                                                              //  2: 500 kHz,
                                                              //  3: Reserved]
#define LORA_SPREADING_FACTOR                       7         // [SF7..SF12]
#define LORA_CODINGRATE                             1         // [1: 4/5,
                                                              //  2: 4/6,
                                                              //  3: 4/7,
                                                              //  4: 4/8]
#define LORA_PREAMBLE_LENGTH                        8         // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT                         0         // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON                  false
#define LORA_IQ_INVERSION_ON                        false
#define RX_TIMEOUT_VALUE                            1000
#define BUFFER_SIZE                                 30 // Define the payload size here

char txpacket[BUFFER_SIZE];
char serial[100];
char rxpacket[BUFFER_SIZE];

static RadioEvents_t RadioEvents;

typedef enum
{
    STATE_RX,
    STATE_TX
}States_t;

States_t state;
int16_t txNumber = 0;
int16_t rssi,rxSize;
bool lora_idle = true;

TaskHandle_t Task0;
TaskHandle_t Task1;

void setup() {

  Serial.begin(115200);
  // Heltec.display->clear();

  Mcu.begin();
  txNumber=0;
  rssi=0;

  RadioEvents.RxDone = OnRxDone;
  RadioEvents.TxDone = OnTxDone;
  RadioEvents.TxTimeout = OnTxTimeout;
  Radio.Init( &RadioEvents );
  Radio.SetChannel( RF_FREQUENCY );
  Radio.SetTxConfig( MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                                LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                                LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                                true, 0, 0, LORA_IQ_INVERSION_ON, 3000 );
  Radio.SetRxConfig( MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                            LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                            LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                            0, true, 0, 0, LORA_IQ_INVERSION_ON, true );
  state=STATE_TX;

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



void SendReceivecode(void* pvParameters) {
  Serial.print("Task1 running on core ");
  Serial.println(xPortGetCoreID());

  //initialize OLED display
  // Heltec.begin(true /*DisplayEnable Enable*/, false /*LoRa Disable*/, true /*Serial Enable*/);
  // Heltec.display->clear();
  // Heltec.display->drawString(0, 0, "LORA Comm Start");
  // Heltec.display->display();


  switch (state)
  {

      case STATE_TX:
      txNumber++;
      sprintf(txpacket,"Hello world number %0.2f",txNumber);  //start a package
      sprintf(serial, "\r\nsending packet, length %d\r\n", strlen(txpacket));
      Serial.printf(serial);

      // display.setFont(ArialMT_Plain_10);
      // display.drawString(0, 10, txpacket);
      // display.setFont(ArialMT_Plain_10);
      // display.drawStringMaxWidth(0, 10, 128,serial);
      // // write the buffer to the display
      // display.display();
      //Heltec.display->drawString(0, 10, txpacket);

      Radio.Send( (uint8_t *)txpacket, strlen(txpacket) ); //send the package out 
      Radio.IrqProcess( );
      break;
      
      case STATE_RX:
      // Heltec.display->clear();
      lora_idle = false;
      Serial.println("into RX mode");

      // display.setFont(ArialMT_Plain_10);
      // display.drawString(0,10, "Waiting for packets...");
      // display.display();
      // Heltec.display->drawString(0,10,"Waiting for packets...");

      Radio.Rx(0);
      Radio.IrqProcess( );
      delay(10000);
      state = STATE_TX;
      break;
      
      default:
      break;
  }

  for (;;) {
    Serial.println("  Core 1 processing - send/receive");
    delay(1000);

  }
}

void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr ) {   
    char result[100];
    rssi=rssi;
    rxSize=size;
    memcpy(rxpacket, payload, size );
    rxpacket[size]='\0';
    Radio.Sleep( );
    sprintf(result,"\r\nreceived packet \"%s\" with rssi %d , length %d\r\n",rxpacket,rssi,rxSize);
    Serial.printf(result);

    // display.setFont(ArialMT_Plain_10);
    // display.drawStringMaxWidth(0,20,128,result);
    // display.display();
    // Heltec.display->drawString(0,20,result);

    state = STATE_TX;
}

void OnTxDone( void ) {
  Serial.println("TX done......");

  // display.setFont(ArialMT_Plain_10);
  // display.drawString(0, 50, "TX done......");
  // // write the buffer to the display
  // display.display();
  // Heltec.display->drawString(0,50,"TX done....");

  state = STATE_RX;
}

void OnTxTimeout( void ) {
    Radio.Sleep( );
    Serial.println("TX Timeout......");

    // display.setFont(ArialMT_Plain_10);
    // display.drawString(0, 50, "TX Timeout......");
    // // write the buffer to the display
    // display.display();
    // Heltec.display->drawString(0,50,"TX timeout......");
}

