/*
 SMS receiver
 
 This sketch, for the Arduino GSM shield, waits for a SMS message 
 and displays it through the Serial port. 
 
 Circuit:
 * GSM shield attached to and Arduino
 * SIM card that can receive SMS messages
 
 created 25 Feb 2012
 by Javier Zorzano / TD
 
 This example is in the public domain.
 
 http://arduino.cc/en/Tutorial/GSMExamplesReceiveSMS
 
*/

// include the GSM library
#include <GSM.h>

// PIN Number for the SIM
#define PINNUMBER ""

// initialize the library instances
GSM gsmAccess;
GSM_SMS sms;
// Array to hold the number a SMS is retreived from
char senderNumber[20];



// DMX Stuff

#include <DmxSimple.h>

int dmxPin = 11;
int maxChannels = 6;
int lightCount = 2;

///////////////////////////////////////////////
int randomLedColour[3]; // for creating a random sequence
int RED[3] = {
  254,0,0};
int GREEN[3] = {
  0,255,0};
int BLUE[3] = {
  0,0,254};
int WHITE[3] = {
  255,255,255};
int OFF[3] = {
  0,0,0};
int currentRGB[3] = {
  0,0,0};
  
  
/// Incoming Payload
int rgb[3];
int value = 0;
int index = 0;
  
  
void setColor(int lightIndex, int* color, int fadeTime=0){
  /* Do a load of really boring array unpacking and channel calculations. */
  int r, g, b;
  r = color[0];
  g = color[1];
  b = color[2];
  int firstChannel = (lightIndex * 3) - 2;
  int rChannel = firstChannel;
  int gChannel = rChannel + 1;
  int bChannel = gChannel + 1;

  DmxSimple.write(rChannel, r);
  DmxSimple.write(gChannel, g);
  DmxSimple.write(bChannel, b);
}

void off(int lightIndex){
  setColor(lightIndex, OFF);
}

void all(int* color) {
  for(int i=1; i < lightCount; i++){
    setColor(i, color);
  }
}

void setup() 
{
  DmxSimple.usePin(dmxPin);
  DmxSimple.maxChannel(maxChannels); // 2 * 3 = 6//  Serial.begin(9600);
  all(OFF);  

  // initialize serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  } 
  
  Serial.println("SMS Messages Receiver");
    
  // connection state
  boolean notConnected = true;
  
  // Start GSM connection
  while(notConnected)
  {
    if(gsmAccess.begin(PINNUMBER)==GSM_READY)
      notConnected = false;
    else
    {
      Serial.println("Not connected");
      delay(1000);
    }
  }
  setColor(1, GREEN);
  delay(200);
  setColor(1, OFF);
  
  Serial.println("GSM initialized");
}

void loop() 
{
  char c;
  
  // If there are any SMSs available()  
  if (sms.available())
  {
    Serial.println("Message received from:");
    
    // Read message bytes and print them
    while(c=sms.read()){
      processPayload(c);
    }
          
    Serial.println("\nEND OF MESSAGE");
    
    // Delete message from modem memory
    sms.flush();
    Serial.println("MESSAGE DELETED");
  }

  delay(500);
}

void pattern(void){
  setColor(2, BLUE);
  delay(200);
  setColor(2, GREEN);  
  delay(200);  
  setColor(2, RED);  
  delay(200);    
  setColor(2, OFF);  
}

void processPayload(int c){

  if ((c>='0') && (c<='9')) {
    value = 10*value + c - '0';
  } 
  else {
    if (c==',' || c=='p'){ 
      rgb[index] = value;
      index ++;
    }
    if (c=='p') {
      handlePayload();
      index = 0;
    }
    value = 0;
  }
}

void handlePayload(void){
    Serial.println('-');
    Serial.print(rgb[0]);
    Serial.print(',');
    Serial.print(rgb[1]);
    Serial.print(',');      
    Serial.print(rgb[2]);
    setColor(1, rgb);
    delay(1000);
    rgb[0] = 0;
    rgb[1] = 0;
    rgb[2] = 0;
}
