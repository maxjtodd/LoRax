#include "LoRaWan_APP.h"
#include <Wire.h>   
#include <ctime>
#include <chrono>
#include "HT_SSD1306Wire.h"
#include "Arduino.h"
#include "LoRa.h"
#include "shared_data.h"

static RadioEvents_t RadioEvents;

#define RF_FREQUENCY                                915000000 // Hz
#define TX_OUTPUT_POWER                             14        // dBm
#define LORA_BANDWIDTH                              0         // [0: 125 kHz, 1: 250 kHz, 2: 500 kHz, 3: Reserved]
#define LORA_SPREADING_FACTOR                       7         // [SF7..SF12]
#define LORA_CODINGRATE                             1         // [1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8]
#define LORA_PREAMBLE_LENGTH                        8         // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT                         0         // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON                  false
#define LORA_IQ_INVERSION_ON                        false

#define RX_TIMEOUT_VALUE                            1000
#define BUFFER_SIZE                                 30 // Define the payload size here

char txpacket[BUFFER_SIZE];
char serial[100];
char rxpacket[BUFFER_SIZE];

extern RadioEvents_t RadioEvents;
enum State
{
    STATE_RX,
    STATE_TX,
    STATE_TX_CONTACT
};
State state;
int16_t txNumber=0;
int16_t pingCount = 0;
int16_t rssi, rxSize;
bool lora_idle=true;

// Need a third type of message - ACK's
// everytime we send a message out, we dont pop from the queue until we receive an ACK that corresponds to the message
int ack_list[10];



// global instance of messageData - contactPing
messageData contactPing = {ESP.getEfuseMac(), NULL, NULL};

void SendReceivecode(void* vpParameters) {


  // display.init();
  // display.setFont(ArialMT_Plain_10);

  Mcu.begin();
  
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

  state=STATE_TX_CONTACT; // default to sending contact ping

  delay(2000); // delay 2s to allow for BLE setup

  for (;;) {
    //Serial.println("  Core 1 processing - send/receive");

    if (messageDataQueue_toLora.queue.size() >= 1) {
      state=STATE_TX; // new message received from bluetooth
    }


    switch (state) {

      // CASE - send contact ping
      case STATE_TX_CONTACT:
      {
        Serial.println("Building Contact Ping");

        // get current time
        // auto now = std::chrono::system_clock::now();
        // std::time_t now_c = std::chrono::system_clock::to_time_t(now);

        // append time to conactPing obj
        // contactPing.messageSentTime = now_c;

        // broadcast ping
        pingCount++;
        Serial.printf("Sending Ping: %d, %d\n", contactPing.id, pingCount);
        sprintf(txpacket, "0;%d", contactPing.id);

        Radio.Send( (uint8_t *)txpacket, strlen(txpacket) );
        Radio.IrqProcess( );
        delay(3000); // delay 2s
      
        break;
      }
      // CASE - message to send from iOS  
      case STATE_TX:
      { // make explicit
        txNumber++; // increment send count
        messageData data_to_send = getMessageData(messageDataQueue_toLora); // get data_to_send to send
        sprintf(txpacket, "LoRa message : id %d , size : %d, value : %s", data_to_send.id, data_to_send.size, data_to_send.value);
        Serial.printf(txpacket);
        Radio.Send( (uint8_t *)txpacket, strlen(txpacket) ); //send the package out 
        Radio.IrqProcess( );
        break;
      }

      // CASE - listen for incoming messages
      case STATE_RX:
        lora_idle = false;
        Serial.println("into RX mode");
        Radio.Rx(0);             // start channel activity detection     
        Radio.IrqProcess( );
        delay(5000); // listen for 5s

        // Send another contact ping
        state = STATE_TX_CONTACT;

        break;
      
      default:
        break;
      
    }

  }

}

void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
{   
    char result[100];                 // array to store resulting string 
    rssi=rssi;
    rxSize=size;
    memcpy(rxpacket, payload, size ); // copy message 
    rxpacket[size]='\0';              // terminating char
    Radio.Sleep( );                 
    sprintf(result,"\r\nreceived packet \"%s\" with rssi %d , length %d\r\n",rxpacket,rssi,rxSize);
    Serial.printf(result);

    // THREE OPTIONS ON MESSAGE RECEIVE 
        //  - Contact Ping - append to BT list w contantPing Designation
        //  - Message      - append to BT list w Message Designation
        //  - Ack of previous message send - pop associated message from LoRa queue

    // Determine Packet type
    if (rxpacket[0] == 0) {
      Serial.println("Contact Ping Received");
      
      // send ack 
    }
    else if (rxpacket[0] == 1) {
      Serial.println("Message Received - %s", payload);
      // create new messageData object, append to list for BT
      messageData data_to_send;
      data_to_send.id = rssi;
      data_to_send.size = size;
      data_to_send.value = rxpacket;

    }

    else {
      // ack from previous message received

    }

    state = STATE_TX;
}

void OnTxDone( void )
{
  Serial.println("\nTX done......Switching to RX");
  state = STATE_RX;
}

void OnTxTimeout( void )
{
    Radio.Sleep( );
    Serial.println("\nTX Timeout......");
}
