#include "LoRaWan_APP.h"
#include <Wire.h>               
#include "HT_SSD1306Wire.h"
#include "Arduino.h"


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

void setup() {
    Serial.begin(115200);

    display.init();
    display.setFont(ArialMT_Plain_10);

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
}



void loop()
{ 
  delay(1000);
  display.clear();
  switch (state)
  {

    case STATE_TX:
      display.clear();     
      txNumber++;
      sprintf(txpacket,"Hello world number %0.2f",txNumber);  //start a package
      sprintf(serial, "\r\nsending packet, length %d\r\n", strlen(txpacket));
      Serial.printf(serial);
      display.setFont(ArialMT_Plain_10);
      display.drawString(0, 10, txpacket);
      display.setFont(ArialMT_Plain_10);
      display.drawStringMaxWidth(0, 10, 128,serial);
      // write the buffer to the display
      display.display();
      Radio.Send( (uint8_t *)txpacket, strlen(txpacket) ); //send the package out 
      Radio.IrqProcess( );
      break;
    
    case STATE_RX:
      display.clear();
      lora_idle = false;
      Serial.println("into RX mode");
      display.setFont(ArialMT_Plain_10);
      display.drawString(0,10, "Waiting for packets...");
      display.display();
      Radio.Rx(0);
      Radio.IrqProcess( );
      delay(10000);
      state = STATE_TX;
      break;
    
    default:
      break;

  }
  
}

void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
{   
    char result[100];
    rssi=rssi;
    rxSize=size;
    memcpy(rxpacket, payload, size );
    rxpacket[size]='\0';
    Radio.Sleep( );
    sprintf(result,"\r\nreceived packet \"%s\" with rssi %d , length %d\r\n",rxpacket,rssi,rxSize);
    Serial.printf(result);
    display.setFont(ArialMT_Plain_10);
    display.drawStringMaxWidth(0,20,128,result);
    display.display();
    state = STATE_TX;
}

void OnTxDone( void )
{
  Serial.println("TX done......");
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 50, "TX done......");
  // write the buffer to the display
  display.display();
  state = STATE_RX;
}

void OnTxTimeout( void )
{
    Radio.Sleep( );
    Serial.println("TX Timeout......");
    display.setFont(ArialMT_Plain_10);
    display.drawString(0, 50, "TX Timeout......");
    // write the buffer to the display
    display.display();
}
