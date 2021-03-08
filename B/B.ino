#include <VirtualWire.h>
#include <avr/wdt.h>


// -----------------------
// RF reception (on 434Mhz)
#define MAXSIGNAL    270
#define MINSIGNAL    550
int signalStrength = -1;
unsigned long lastSignal = 0;
const unsigned long timeout = 5000; // timeout (in millisecs) after which other object will be considered OFF
const unsigned long resetTimeout = 10000;
long signalsToSum = 4; // creates an average signal strength based on 4 pings received
long signalsCount = 0;
long signalsSum = 0;
long averageSignalStrength = -1;

// -----------------------
// RF transmission (on 434Mhz)
unsigned long lastTransmission = 0;
const unsigned long transmissionTimeout = 500; 


// -----------------------
// LIGHT
int ledPin = 3; 
int lightMode = 0;
int lightTimer = 15;
 
 
bool bDebug = true;
 
// -----------------------
void setup()
{
    Serial.begin(9600); // Debugging only
    Serial.println("B");
 
    // Initialise the IO and ISR
    vw_set_rx_pin(2);
    vw_set_tx_pin(4); 
    vw_set_ptt_inverted(true); // Required for DR3100
    vw_setup(2000);      // Bits per sec
    vw_rx_start();    
}



// ----------------------- 
void loop()
{
   receiveRF();
   if ((millis() - lastTransmission) >= transmissionTimeout) {
     transmitRF();
     lastTransmission = millis();
   }
   
   
   // other object still alive?
   //Serial.println(millis() - lastSignal);
   if ((millis() - lastSignal) >= timeout) {
     if (lightMode > 0) {
       lightMode = 0;
       averageSignalStrength = -1;
       Serial.println("other object is down");
     }
   } 

  if ((millis() - lastSignal) >= resetTimeout) {
    Serial.println("reboot");
    reboot();
  }
   
   //Serial.println(signalStrength);
   
   
   
   if (averageSignalStrength > -1) {
     //lightMode = 5;
     lightMode = map(averageSignalStrength, MAXSIGNAL, MINSIGNAL, 5, 0);
     lightMode = constrain(lightMode, 0, 5);
   }
   
   
   light();
}


// -----------------------
void receiveRF() {
    uint8_t buf[VW_MAX_MESSAGE_LEN];
    uint8_t buflen = VW_MAX_MESSAGE_LEN;
    
    //Serial.println(analogRead(A0));
       
    if (vw_get_message(buf, &buflen)) // Non-blocking
    {
        
        lastSignal = millis();
        signalStrength = analogRead(A0);
        
        if (bDebug == true) {
          uint8_t sum;        
          int i;
 
          // Message with a good checksum received, dump it.
        
          Serial.print("Received: [");
       
          for (i = 0; i < buflen; i++)
          {
              Serial.print(buf[i], HEX);
              sum += buf[i];
          }
        
          Serial.print("] strength ");
          Serial.println(signalStrength);
          Serial.println(sum);
        }
      
      
      if (signalsCount < signalsToSum) {
        signalsSum += signalStrength;
        signalsCount ++;
      } else if (signalsCount == signalsToSum) {
        averageSignalStrength = (signalsSum / signalsToSum);
        Serial.print ("average signal strength: ");
        Serial.println (averageSignalStrength);
        signalsCount = 0;
        signalsSum = 0;
      }
   }
}



// -----------------------
void transmitRF() {
    char *msg = "B";
    
    vw_send((uint8_t *)msg, strlen(msg));
    vw_wait_tx(); // Wait until the whole message is gone
    Serial.println(msg);
}


// -----------------------
void light() {
  uint32_t ledValue;
  
  if (lightMode == 0) {
    ledValue = (uint32_t) random(0, 200);
  } else if (lightMode == 1) {
    ledValue = (uint32_t) (128+127*cos(2*PI/5000*millis()));
  } else if (lightMode == 2) {
    ledValue =  (uint32_t) (128+127*cos(2*PI/3000*millis()));
  } else if (lightMode == 3) {
    ledValue =  (uint32_t) (128+127*cos(2*PI/1500*millis()));
  } else if (lightMode == 4) {
    ledValue =  (uint32_t) (128+127*cos(2*PI/500*millis()));
  } else if (lightMode == 5) {
    ledValue =  (uint32_t) (128+127*cos(2*PI/10000*millis()));
  }
    
  analogWrite(ledPin, ledValue);
  
}

void reboot() {
  wdt_enable(WDTO_15MS);
  while(1)
  {
  }
}
