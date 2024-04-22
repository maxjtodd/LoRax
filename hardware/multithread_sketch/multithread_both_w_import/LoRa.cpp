#include "LoRaWan_APP.h"
#include <Wire.h>   
#include <ctime>
#include <chrono>
#include <string>
#include <iostream>
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
int16_t sendMessageCount = 0;
int16_t rssi, rxSize;
bool lora_idle=true;

// Need a third type of message - ACK's
// everytime we send a message out, we pop the message from original queue, add to ack_wait_list
// we only need unique id of message, list of char arrays
char* sentMessageIDs[100];

const int ident_message = 0x01;
const int ident_contact = 0x02;
const int ident_ack = 0x03;


// global instance of messageData - contactPing
messageData contactPing = {ESP.getEfuseMac(), NULL, NULL};


messageData processMessageString(char* message) {
  /*
  Params : (char*) message
  
    Process a message (char*)
    Return a new messageData object.
    Attributes assigned to those passed in message

    MESSAGE FORMAT:

      TYPE;ID:CONTENT;
  
  */

  int type_token;
  char* id_token;
  char* message_token;
  char* dest_token;

  std::string delimiter = ";";
  std::string s = message;

  size_t pos = 0;
  int count = 0;
  std::string token;
  // iterate thru string, separate by ';', assign each token
  while ((pos = s.find(delimiter)) != std::string::npos) {
      token = s.substr(0, pos);
      char* token_char = new char[token.length()+1];
      strcpy(token_char, token.c_str());
      //Serial.printf("   TOKEN: %s\n", token_char);
      s.erase(0, pos + delimiter.length());
      
      if (count == 0) { type_token = strtol(token_char, &id_token, 10); }
      else if (count == 1) { id_token = token_char; }
      else if (count == 2) { dest_token = token_char; }
      else { message_token = token_char; }
      count++;
  }

  messageData newMessage = {type_token, id_token, dest_token, 0, message_token };

  return newMessage;
} // processMessageString

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

        // ping printout
        pingCount++;
        Serial.printf("Sending Ping: %d, %d\n", contactPing.message_id, pingCount);
        sprintf(txpacket, "0;%d;0;0;0", contactPing.message_id);

        // send contact ping
        Radio.Send( (uint8_t *)txpacket, strlen(txpacket) );
        Radio.IrqProcess( );
      
        delay(3000); // delay 3s 

        break;
      }
      // CASE - message to send from iOS  
      case STATE_TX:
      { // make explicit

        // increment send count
        sendMessageCount++;

        // pop message packet from waiting queue
        messageData data_to_send = getMessageData(messageDataQueue_toLora);


        // packet prinout
        Serial.printf("PACKET FROM BT : %d;%s;%s;%d;%s\n", data_to_send.message_type, data_to_send.message_id, data_to_send.message_dest,  data_to_send.size, data_to_send.value);
        sprintf(txpacket, "%d;%s;%s;%d;%s", data_to_send.message_type, data_to_send.message_id, data_to_send.message_dest,  data_to_send.size, data_to_send.value);
        Serial.printf(txpacket);

        Serial.println("\nSending...");
        // send the packet out
        Radio.Send( (uint8_t *)txpacket, strlen(txpacket) ); 
        Radio.IrqProcess( );
        Serial.println("Sent.");

        // // add packet ID to ack llst
        // size_t size = sizeof(sentMessageIDs);
        // sentMessageIDs[size] = data_to_send.message_id;
        state = STATE_RX;

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


void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr ) {   
    memcpy(rxpacket, payload, size ); // copy message 
    rxpacket[size]='\0';              // terminating char
    Radio.Sleep( );                   

    // build serial monitor printout
    char result[100];                             
    sprintf(result,"\r\nreceived packet \"%s\", length %d\r\n",rxpacket,rxSize);
    Serial.printf(result);

    // THREE OPTIONS ON MESSAGE RECEIVE 
        //  - Contact Ping - append to BT list w contantPing Designation
        //  - Message      - append to BT list w Message Designation
        //  - Ack of previous message send - pop associated message from LoRa queue
    
    /* PCKAET STRUCTURE

          delim = ;

      TYPE;MESSAGENUM_MACADDR;DEST_MACADDR;message_contant;

    */

    messageData newMessage = processMessageString(result);
    Serial.printf(" New Message from LoRa:\n    Type:%d\n   ID:%d\n   Content:%d", newMessage.message_type, newMessage.message_id, newMessage.value);


    // Determine Packet type

    // contact ping
    if (newMessage.message_type == 0) {

      // Send contact device MAC ADDR to iOS app, determine if seen before
      Serial.println("Type is Contant Ping - push to BT Queue");

      pushMessageData(messageDataQueue_toBT, newMessage);

      

    }

    // new message
    else if (newMessage.message_type == 1) {

      Serial.println("Type is Message - push to BT Queue");
      
      pushMessageData(messageDataQueue_toBT, newMessage);

      // build, send ack
      messageData ack = {2, newMessage.message_id, 0, 0, 0};
      pushMessageData(messageDataQueue_toLora, ack);

    }

    // ack from previous sent message
    else {
      Serial.println("Type is ACK");

      // // check ack list n
      // int size = sizeof(sentMessageIDs) / sizeof(sentMessageIDs[0]);
      // for (int i = 0; i < size; i++) {
        
      //   // if ack matches previously sent ID
      //   if strcmp(newMessage.message_ID, sentMessageIDs[i] == 0) {
          
      //     // send ack info to iOS


      //   }

      // }

      pushMessageData(messageDataQueue_toBT, newMessage);

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
