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
#define TX_OUTPUT_POWER                             20        // dBm
#define LORA_BANDWIDTH                              1         // [0: 125 kHz, 1: 250 kHz, 2: 500 kHz, 3: Reserved]
#define LORA_SPREADING_FACTOR                       9        // [SF7..SF12]
#define LORA_CODINGRATE                             1         // [1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8]
#define LORA_PREAMBLE_LENGTH                        8         // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT                         0         // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON                  false
#define LORA_IQ_INVERSION_ON                        false

#define RX_TIMEOUT_VALUE                            1000
#define BUFFER_SIZE                                 50 // Define the payload size here

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


// forward declarations
void sendContactPing();
void sendMessageFromiOS(); 
messageData processMessageString(char * message);



// global instance - contanct ping linked list
LL* knownContacts = new LL;

// global intance - sent messages linked list
LL* sentMessageIDs = new LL;


// global instance of messageData - contactPing
messageData contactPing = {ESP.getEfuseMac(), NULL, NULL};

char* get_mac_address_static() {
    static char mac_addr[18];  // Static buffer to hold the MAC address
    uint8_t baseMac[6];

    if (esp_read_mac(baseMac, ESP_MAC_BT) == ESP_OK) {
        sprintf(mac_addr, "%02x:%02x:%02x:%02x:%02x:%02x", 
                baseMac[0], baseMac[1], baseMac[2], 
                baseMac[3], baseMac[4], baseMac[5]);
        return mac_addr;
    } else {
        return NULL;  // Return NULL if the MAC address could not be read
    }
}



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



  /*                             */
  /*        INFINITE LOOP        */
  /*                             */

  for (;;) {

    // CHECK FOR NEW MESSAGE FROM BT_CORE 
    if (messageDataQueue_toLora.queue.size() >= 1) {
      state=STATE_TX; // new message received from bluetooth
    }

    // CHECK FOR ANY MESSAGES THAT HAVE NOT BE ACK'd  


    // if - 
    // we have a message that has not been ack'ed
    // we send one more time, then swith to RECEIVE to listen for ACK
    // PROBLEM - do we send for every message, then listen?

    
  /*                             */
  /*        CONTROL SWITCH       */
  /*                             */
    switch (state) {



      // CASE - send contact ping
      case STATE_TX_CONTACT:
      {
        //Serial.println("Building Contact Ping");
  
        sendContactPing(); // call send contact ping
      
        delay(1000); // delay 3s 

        state = STATE_RX; // listen mode 
        break;
      }



      // CASE - message to send from iOS  
      case STATE_TX:
      { 
 
        sendMessageFromiOS(); // call send message from iOS

        delay(1000);

        state = STATE_RX;
        break;
      }



      // CASE - listen for incoming messages
      case STATE_RX:

        lora_idle = false;
        Serial.println("into RX mode");
        Radio.Rx(0);             // start channel activity detection     
        Radio.IrqProcess( );
        delay(7500); // listen for 7.5s

        // Send another contact ping
        state = STATE_TX_CONTACT;

        break;
      
      default:
        break;
      
    }

  }

}

messageData processMessageString(char* message) {
    /*
    Params: char* message

    Process a message (char*)
    Return a new messageData object.
    Attributes assigned to those passed in message

    MESSAGE FORMAT:

      TYPE;send_ID;recpt_ID;size;CONTENT
    */
    
    const char* delimiter = ";";
    char* token;
    int count = 0;
    
    messageData newMessage = {0, nullptr, nullptr, 0, nullptr};
    
    // Use strtok to split the string by ';'
    token = strtok(message, delimiter);
    while (token != nullptr) {
        if (count == 0) {
            newMessage.message_type = atoi(token); // Convert first token to integer for type
        } else if (count == 1) {
            newMessage.message_id= strdup(token); // Duplicate the sender ID token
        } else if (count == 2) {
            newMessage.message_dest = strdup(token); // Duplicate the recipient ID token
        } else if (count == 3) {
          newMessage.size = atoi(token);
        } else if (count == 4) {
            newMessage.value = strdup(token); // Duplicate the content token
            break; // No need to process further tokens
        }
        count++;
        token = strtok(nullptr, delimiter); // Continue splitting the string
    }

    return newMessage;
}

// From RXDone
void handleIncomingMessage(messageData newMessage) {
    // THREE OPTIONS ON MESSAGE RECEIVE 
        //  - Contact Ping - append to BT list w contantPing Designation
        //  - Message      - append to BT list w Message Designation
        //  - Ack of previous message send - pop associated message from LoRa queue
    
    /* PCKAET STRUCTURE

          delim = ;

      TYPE;MESSAGENUM_MACADDR;DEST_MACADDR;message_contant;

    */

  switch (newMessage.message_type) {

    // CONTACT PING
    case 0:
    {

      // New pping received
      Serial.println("Type is Contant Ping - check if previous contact");

      // Determine if ping has been received from this contact before. 
      if (knownContacts->contains(newMessage.message_id)) {
        Serial.println("Contact has been previously recorded");
      }

      // if not seen before, add to seen and push to iOS as new contact. 
      else {
        Serial.println("New Contact. Pushing to BT.");
        // insert into linked list
        knownContacts->insert(newMessage.message_id);
        // insert into queue
        pushMessageData(messageDataQueue_toBT, newMessage);

      }

      break;
    }

    // NEW MESSAGE
    case 1:
    {

      Serial.println("Type is Message - push to BT Queue");
      
      pushMessageData(messageDataQueue_toBT, newMessage);

      Serial.println("Build and send ACK");

      // build ack message
      char* a = "";
      Serial.printf("   ACK: %d;%s;%s;%d;%d", newMessage.message_type, newMessage.message_id, newMessage.message_dest, newMessage.size, newMessage.value);

      // add to queue to be sent
      pushMessageData(messageDataQueue_toLora, newMessage);

      break;
    }

    // ACKKNOWLEDGEMENT 
    case 2:
    {

      Serial.println("Type is ACK");

      // iterate thru linked list of previously sent messages
      // find sent messages corresponding to ACK
      if (sentMessageIDs->contains(newMessage.message_id)) {
        
        // remove from linked list - do not send any more repeats
        sentMessageIDs->remove(newMessage.message_id);
      }

      break;

    default:
      break;
    }
  }
}

// From Control Switch
void sendContactPing() {

  char* mac_addr = get_mac_address_static();

  // ping printout
  pingCount++;
  Serial.printf("Sending Ping: %s, %d\n", mac_addr, pingCount);

  // build send addr
  
  
  sprintf(txpacket, "0;%d_%s;0;0;0", pingCount, mac_addr);

  // send contact ping
  Radio.Send( (uint8_t *)txpacket, strlen(txpacket) );
  Radio.IrqProcess( );

}

// From Control Switch
void sendMessageFromiOS() {

  // increment send count
  sendMessageCount++;

  // pop message packet from waiting queue
  messageData data_to_send = getMessageData(messageDataQueue_toLora);


  // packet prinout
  Serial.printf("\n   PACKET FROM BT (to be sent) : %d;%d_%s;%s;%d;%s\n", data_to_send.message_type, txNumber, data_to_send.message_id, data_to_send.message_dest,  data_to_send.size, data_to_send.value);
  sprintf(txpacket, "%d;%s;%s;%d;%s", data_to_send.message_type, data_to_send.message_id, data_to_send.message_dest,  data_to_send.size, data_to_send.value);
  Serial.printf(txpacket);

  if (txpacket != nullptr) {
      Radio.Send((uint8_t*)txpacket, strlen(txpacket));
      Radio.IrqProcess( );
      Serial.println("\nSent. (from BT)\n");

      delay(1500);

      // add packet identifier to sent messages
      Serial.println("Inserting into sent messages");
      sentMessageIDs->insert(data_to_send.message_id);

      // increment txnumb
      txNumber++;
  } else {
      Serial.println("Error: txpacket is null.");
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

    // build messageData object
    messageData newMessage = processMessageString(rxpacket);
    Serial.printf(" New Message from LoRa:\n    Type:%d\n   ID:%s\n   Content:%s\n", newMessage.message_type, newMessage.message_id, newMessage.value);


    // Handle message based on type
    handleIncomingMessage(newMessage);

    // switch to send mode
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
