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
#include "AES.h"  

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

// Constants for the key exchange protocol
#define PRIME_MODULUS       32749 // A prime number smaller than 32767
#define BASE_GENERATOR      5

#define DEBUG                                       1

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

uint8_t key[16];
uint8_t iv[16];

bool lora_idle=true;
bool sharedKey=false;

int private_key;
int public_key;

int paddedLen = 0;
byte *decryptedMessage = NULL;
byte *paddedMessage = NULL;

AES aes;

// Need a third type of message - ACK's
// everytime we send a message out, we pop the message from original queue, add to ack_wait_list
// we only need unique id of message, list of char arrays
char* sentMessageIDs[100];

const int ident_message = 0x01;
const int ident_contact = 0x02;
const int ident_ack = 0x03;


// global instance of messageData - contactPing
messageData contactPing = {ESP.getEfuseMac(), NULL, NULL};

void generate_random_iv(uint8_t iv[]) {
  for (size_t i = 0; i < 16; i++) {
    iv[i] = rand() % 256; // Generates a random number between 0 and 255
  }
}

// Function to perform modular exponentiation
int modExp(int base, int exponent, int modulus) {
  int result = 1;
  base = base % modulus; // Ensure base is within the range of modulus

  // Iterate through the bits of the exponent
  while (exponent > 0) {
    // Check if the least significant bit of the exponent is set (odd)
    if (exponent & 1) {
      // Multiply result by base and take modulus with respect to modulus
      result = (result * base) % modulus;
    }

    // Right-shift the exponent by 1 bit to process the next bit
    exponent = exponent >> 1;

    // Square the base and take modulus with respect to modulus
    base = (base * base) % modulus;
  }

  return result; // Return the final result after iterating through all bits of the exponent
}

void performKeyExchange() {
  // Generate private key
  private_key = random(1, PRIME_MODULUS);
  Serial.print("Private Key: ");
  Serial.println(private_key);

  // Calculate public key
  public_key = modExp(BASE_GENERATOR, private_key, PRIME_MODULUS);
  Serial.print("Public Key: ");
  Serial.println(public_key);
  while (!sharedKey) {
    // Send public key to the other device
    sprintf(txpacket, "%d", public_key);
    Radio.Send((uint8_t *)txpacket, strlen(txpacket));
    Radio.IrqProcess();

    // Receive public key from the other device
    delay(100); // Wait for the other device to send its public key
    Radio.Rx(0);
    Radio.IrqProcess();
  }
}

void deriveEncryptionKey(int received_public_key) {
    // Derive shared secret using modular exponentiation
    int shared_secret = modExp(received_public_key, private_key, PRIME_MODULUS);

    // Convert shared_secret to a byte array (encryption key)
    for (int i = 0; i < 16; i++) {
        // Extract each byte of the shared secret and store it in the encryption key array
        // Shift the bits of shared_secret to the right by (8 * i) to extract each byte
        // Then, mask out the least significant byte (8 bits) using bitwise AND with 0xFF
        // Finally, cast the result to a byte type and store it in the encryption key array
        key[i] = (byte)(shared_secret >> (8 * i) & 0xFF);
    }
}

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

  performKeyExchange();

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
        byte *paddedTxPacket;
        // pop message packet from waiting queue
        messageData data_to_send = getMessageData(messageDataQueue_toLora);

        // packet prinout
        Serial.printf("PACKET FROM BT : %d;%s;%s;%d;%s\n", data_to_send.message_type, data_to_send.message_id, data_to_send.message_dest,  data_to_send.size, data_to_send.value);
        sprintf(txpacket, "%d;%s;%s;%d;%s", data_to_send.message_type, data_to_send.message_id, data_to_send.message_dest,  data_to_send.size, data_to_send.value);
        Serial.printf(txpacket);

        // Encrypt packet
            paddedLen = aes.get_padded_len(strlen(txpacket));
            byte *paddedMessage = (byte *)malloc(paddedLen);
            if (paddedMessage == NULL) {
                Serial.println("Memory allocation failed");
                return;
            }
            memset(paddedMessage, 0, paddedLen); // Initialize with 0
            memcpy(paddedMessage, txpacket, strlen(txpacket)); // Copy the message
            byte *paddedMessagePtr = paddedMessage; // Store the pointer to pass it to the padPlaintext function
            aes.padPlaintext((const void*)paddedMessagePtr, paddedMessage); // Corrected argument
            paddedTxPacket = (byte *)malloc(paddedLen);
            if (paddedTxPacket == NULL) {
                Serial.println("Memory allocation failed");
                free(paddedMessage); // Free allocated memory
                return;
            }
            aes.cbc_encrypt(paddedMessage, paddedTxPacket, paddedLen / 16, iv);
            free(paddedMessage); // Free allocated memory


        Serial.println("\nSending...");
        // send the packet out
        Radio.Send( paddedTxPacket, paddedLen ); 
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
    
    if(!sharedKey) {
      
      // If shared key is not yet established, received payload is assumed to be the public key
      int received_public_key = atoi((char*)payload); // Convert payload to integer
      
      // Derive encryption key from received public key
      deriveEncryptionKey(received_public_key);

      // Set sharedKey to true to indicate that key exchange is completed
      sharedKey = true;
      if (DEBUG) {
        //Serial.println("I am here");
        Serial.println(received_public_key);
        
      }
    }

    else{
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
