/*
 
 SMS reciever sends out DMX signal to an RGB Light
 on DMX channels 1,2 and 3
 and for an onboard LED on PWM pins 5,6 and 9
 
*/

// GSM Setup
#include <GSM.h>

// PIN Number for the SIM
#define PINNUMBER ""

GSM gsmAccess;
GSM_SMS sms;
// Array to hold the number a SMS is retreived from
char senderNumber[20];

// DMX Stuff
#include <DmxSimple.h>

int dmxPin = 11; // Cant use 3, as the GSM shield needs it
int maxChannels = 6;
int lightCount = 2;

// Some constants
int RED[3] = {
  254,0,0};
int GREEN[3] = {
  0,254,0};
int BLUE[3] = {
  0,0,254};
int WHITE[3] = {
  254,254,254};
int OFF[3] = {
  0,0,0};
int currentRGB[3] = {
  0,0,0};
  
// PWM Outputs for testing with LED strip
#define REDPIN 5
#define GREENPIN 9
#define BLUEPIN 6
  
  
/// Incoming Payload
int rgb[3];
int value = 0;
int index = 0;
  
  
void setColor(int lightIndex, int* color, int fadeTime=0){
  int r, g, b;
  r = color[0];
  g = color[1];
  b = color[2];
  int firstChannel = (lightIndex * 3) - 2;
  int rChannel = firstChannel;
  int gChannel = rChannel + 1;
  int bChannel = gChannel + 1;
  // Send DMX Signal
  DmxSimple.write(rChannel, r);
  DmxSimple.write(gChannel, g);
  DmxSimple.write(bChannel, b);
  // PWM RGB signal to pins for debug LEDs
  analogWrite(REDPIN, r);
  analogWrite(GREENPIN, g);
  analogWrite(BLUEPIN, b);
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
  // RGB LEDS
  pinMode(REDPIN, OUTPUT);
  pinMode(GREENPIN, OUTPUT);
  pinMode(BLUEPIN, OUTPUT);

  // DMX Shield
  DmxSimple.usePin(dmxPin);
  DmxSimple.maxChannel(maxChannels); // 2 * 3 = 6
  all(OFF);
  setColor(1, OFF);

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
  
  setColor(1, RED);
  delay(1000);
  setColor(1, GREEN);
  delay(1000);  
  setColor(1, BLUE);
  delay(1000);    
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
    
    // Read message bytes and set the RGB variable
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

// Converts a string of 000,000,000p into RGB char array
// 'p' delineates the end of the payload.
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
    setColor(1, rgb);
    delay(1000);
    rgb[0] = 0;
    rgb[1] = 0;
    rgb[2] = 0;
}

