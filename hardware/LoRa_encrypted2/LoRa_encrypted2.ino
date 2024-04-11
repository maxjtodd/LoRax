#include "LoRaWan_APP.h"
#include <Wire.h>
#include "Arduino.h"
#include "AES.h"  


// Constants for the key exchange protocol
#define PRIME_MODULUS       32749 // A prime number smaller than 32767
#define BASE_GENERATOR      5

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


#define RX_TIMEOUT_VALUE                            2000
#define BUFFER_SIZE                                 30 // Define the payload size here
#define ENCRYPTED_BUFFER_SIZE                       64
#define DEBUG                                       1

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

float txNumber = 0.00;
int paddedLen = 0;
int16_t rssi,rxSize;
byte *decryptedMessage = NULL;
byte *paddedMessage = NULL;

bool lora_idle = true;
bool sharedKey = false;
int private_key;
int public_key;

uint8_t key[16];
uint8_t iv[] = {0x8E, 0x73, 0xB0, 0xF7, 0xDA, 0x0E, 0x64, 0x52, 0xC8, 0x10, 0xF3, 0x2B, 0x80, 0x90, 0x79, 0xE5}; 

AES aes;

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
      // Receive public key from the other device
      Radio.Rx(0);
      Radio.IrqProcess();
      
      // Send public key to the other device
      sprintf(txpacket, "%d", public_key);
      Radio.Send((uint8_t *)txpacket, strlen(txpacket));
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


void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr) {
    if (sharedKey) {
      byte *decryptedMessage = (byte *)malloc(size);
      if (decryptedMessage == NULL) {
        Serial.println("Memory allocation failed");
        return;
      }

      aes.cbc_decrypt(payload, decryptedMessage, size / 16, iv);

      // Print decrypted message 
      Serial.print("Decrypted message: ");
      for (int i = 0; i < size; i++) {
        Serial.print((char)decryptedMessage[i]); 
      }
      Serial.println();
    
      char result[100];
      sprintf(result, "\r\nreceived packet \"%s\" with rssi %d, length %d\r\n", decryptedMessage, rssi, size);
      Serial.printf(result);

      free(decryptedMessage);
    }
    else {
      
      // If shared key is not yet established, received payload is assumed to be the public key
      int received_public_key = atoi((char*)payload); // Convert payload to integer
      
      // Derive encryption key from received public key
      deriveEncryptionKey(received_public_key);

      // Set sharedKey to true to indicate that key exchange is completed
      sharedKey = true;
      if (DEBUG) {
        Serial.println("I am here");
        Serial.println(received_public_key);
        
      }
    }
}

void OnTxDone() {
    Serial.println("TX done......");
}

void OnTxTimeout() {
    Radio.Sleep();
    Serial.println("TX Timeout......");
}

void setup() {
    Serial.begin(115200);
    Mcu.begin();

    randomSeed(analogRead(0));

    if (DEBUG){
      for (int i = 0; i < 10; i++) {
        int randNum = random(1, PRIME_MODULUS);
        Serial.print("Random number ");
        Serial.print(i + 1);
        Serial.print(": ");
        Serial.println(randNum);
      }
    }
    RadioEvents.RxDone = OnRxDone;
    RadioEvents.TxDone = OnTxDone;
    RadioEvents.TxTimeout = OnTxTimeout;

    Radio.Init(&RadioEvents);
    Radio.SetChannel(RF_FREQUENCY);

    Radio.SetTxConfig(MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                      LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                      LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                      true, 0, 0, LORA_IQ_INVERSION_ON, 3000);
    Radio.SetRxConfig(MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                      LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                      LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                      0, true, 0, 0, LORA_IQ_INVERSION_ON, true);
    
    performKeyExchange();

    state = STATE_RX;
    //deriveEncryptionKey();
    //sharedKey = true;
}

void loop() {
    delay(1000);

    switch (state) {
        case STATE_RX:
            
            // Set radio to receive mode
            Radio.Rx(0);
            Radio.IrqProcess();
            Serial.println("Switched to RX mode");
            delay(5000);
            state = STATE_TX; // Change state to transmit mode for the next iteration
            break;

        case STATE_TX: {

            Serial.println("Switched to TX mode");
            
            char txpacket[BUFFER_SIZE];
            char serial[100];
            byte *paddedTxPacket;
            int paddedLen;

            // Generate packet
            sprintf(txpacket, "Hello world number %0.2f", txNumber);
            txNumber += 0.01;
            sprintf(serial, "\rsending packet: %s\nLength %d\r\n", txpacket, strlen(txpacket));
            Serial.printf(serial);

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

            // Print encrypted message
            Serial.print("Encrypted message: ");
            for (int i = 0; i < paddedLen; i++) {
                Serial.print(paddedTxPacket[i], HEX);
                Serial.print(" ");
            }
            Serial.println();

            // Decrypt the transmitted message for testing
            decryptedMessage = (byte *)malloc(paddedLen);
            aes.cbc_decrypt(paddedTxPacket, decryptedMessage, paddedLen / 16, iv);

            if (DEBUG) {
                // Print decrypted message 
                Serial.print("Decrypted message: ");
                //int originalLen = strlen((char*)decryptedMessage) - (aes.get_unpadded_len(decryptedMessage, paddedLen));
                for (int i = 0; i < strlen((char*)decryptedMessage); i++) {
                    Serial.print((char)decryptedMessage[i]); // Assuming decrypted message is text
                }
                Serial.println();
                //Serial.println((char*)decryptedMessage); // Assuming decrypted message is text
            }
            
            // Transmit packet
            Radio.Send(paddedTxPacket, paddedLen); 
            Radio.IrqProcess();
            free(paddedTxPacket);
            free(decryptedMessage);

            
            state = STATE_RX; // Change state to receive mode for the next iteration
            break;
        }

        default:
            // Handle unexpected state
            Serial.println("Unexpected state");
            break;
    }
}
