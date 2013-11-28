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
#include <Tween.h>

int dmxPin = 11; // Cant use 3, as the GSM shield needs it
int maxChannels = 6;
int lightCount = 2;


// Output
// PWM Outputs for testing with LED strip
int redPin = 6;
int grnPin = 9;
int bluPin = 5;


// Color arrays
int black[3]  = { 0, 0, 0 };
int white[3]  = { 100, 100, 100 };
int red[3]    = { 100, 0, 0 };
int green[3]  = { 0, 100, 0 };
int blue[3]   = { 0, 0, 100 };
int yellow[3] = { 40, 95, 0 };
int dimWhite[3] = { 30, 30, 30 };
// etc.

// Set initial color
int redVal = black[0];
int grnVal = black[1]; 
int bluVal = black[2];

int wait = 10;      // 10ms internal crossFade delay; increase for slower fades
int hold = 0;       // Optional hold when a color is complete, before the next crossFade
int DEBUG = 1;      // DEBUG counter; if set to 1, will write values back via serial
int loopCount = 60; // How often should DEBUG report?
int repeat = 3;     // How many times should we loop before stopping? (0 for no stop)
int j = 0;          // Loop counter for repeat

// Initialize color variables
int prevR = redVal;
int prevG = grnVal;
int prevB = bluVal;
    
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
  analogWrite(redPin, r);
  analogWrite(grnPin, g);
  analogWrite(bluPin, b);
}

void off(int lightIndex){
  setColor(lightIndex, black);
}

void all(int* color) {
  for(int i=1; i < lightCount; i++){
    setColor(i, color);
  }
}

void setup() 
{
  // RGB LEDS
  pinMode(redPin, OUTPUT);
  pinMode(grnPin, OUTPUT);
  pinMode(bluPin, OUTPUT);

  // DMX Shield
  DmxSimple.usePin(dmxPin);
  DmxSimple.maxChannel(maxChannels); // 2 * 3 = 6
  all(black);
  
  if (DEBUG) {
    Serial.begin(9600);
    while (!Serial) {
      ; // wait for serial port to connect. Needed for Leonardo only
    } 
  
    Serial.println("SMS Messages Receiver");
  }
    
  // connection state
  boolean notConnected = true;
  
  // Start GSM connection
  while(notConnected)
  {
    if(gsmAccess.begin(PINNUMBER)==GSM_READY)
      notConnected = false;
    else
    {
      if (DEBUG) {
        Serial.println("Not connected");
      }
      delay(1000);
    }
  }
  
  setColor(1, red);
  delay(200);
  setColor(1, green);
  delay(200);  
  setColor(1, blue);
  delay(200);
  setColor(1, black);

  if (DEBUG) {
    Serial.println("GSM initialized");
  }
}

void loop() 
{
  char c;
  
  // If there are any SMSs available()  
  if (sms.available())
  {
    if (DEBUG) {
      Serial.println("Message received from:");
    }
    
    // Read message bytes and set the RGB variable
    while(c=sms.read()){
      processPayload(c);
    }
            
    // Delete message from modem memory
    sms.flush();
    if (DEBUG) {
      Serial.println("MESSAGE DELETED");
    }
  }

  delay(500);
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
    crossFade(rgb);
    delay(1000);
    rgb[0] = 0;
    rgb[1] = 0;
    rgb[2] = 0;
}



/* BELOW THIS LINE IS THE MATH -- YOU SHOULDN'T NEED TO CHANGE THIS FOR THE BASICS
* 
* The program works like this:
* Imagine a crossfade that moves the red LED from 0-10, 
*   the green from 0-5, and the blue from 10 to 7, in
*   ten steps.
*   We'd want to count the 10 steps and increase or 
*   decrease color values in evenly stepped increments.
*   Imagine a + indicates raising a value by 1, and a -
*   equals lowering it. Our 10 step fade would look like:
* 
*   1 2 3 4 5 6 7 8 9 10
* R + + + + + + + + + +
* G   +   +   +   +   +
* B     -     -     -
* 
* The red rises from 0 to 10 in ten steps, the green from 
* 0-5 in 5 steps, and the blue falls from 10 to 7 in three steps.
* 
* In the real program, the color percentages are converted to 
* 0-255 values, and there are 1020 steps (255*4).
* 
* To figure out how big a step there should be between one up- or
* down-tick of one of the LED values, we call calculateStep(), 
* which calculates the absolute gap between the start and end values, 
* and then divides that gap by 1020 to determine the size of the step  
* between adjustments in the value.
*/

int calculateStep(int prevValue, int endValue) {
  int step = endValue - prevValue; // What's the overall gap?
  if (step) {                      // If its non-zero, 
    step = 1020/step;              //   divide by 1020
  } 
  return step;
}

/* The next function is calculateVal. When the loop value, i,
*  reaches the step size appropriate for one of the
*  colors, it increases or decreases the value of that color by 1. 
*  (R, G, and B are each calculated separately.)
*/

int calculateVal(int step, int val, int i) {

  if ((step) && i % step == 0) { // If step is non-zero and its time to change a value,
    if (step > 0) {              //   increment the value if step is positive...
      val += 1;           
    } 
    else if (step < 0) {         //   ...or decrement it if step is negative
      val -= 1;
    } 
  }
  // Defensive driving: make sure val stays in the range 0-255
  if (val > 255) {
    val = 255;
  } 
  else if (val < 0) {
    val = 0;
  }
  return val;
}

/* crossFade() converts the percentage colors to a 
*  0-255 range, then loops 1020 times, checking to see if  
*  the value needs to be updated each time, then writing
*  the color values to the correct pins.
*/

void crossFade(int color[3]) {
  // Convert to 0-255
  int R = (color[0] * 255) / 100;
  int G = (color[1] * 255) / 100;
  int B = (color[2] * 255) / 100;

  int stepR = calculateStep(prevR, R);
  int stepG = calculateStep(prevG, G); 
  int stepB = calculateStep(prevB, B);

  for (int i = 0; i <= 1020; i++) {
    redVal = calculateVal(stepR, redVal, i);
    grnVal = calculateVal(stepG, grnVal, i);
    bluVal = calculateVal(stepB, bluVal, i);
    // Set new colour to PWM pins and send DMX signal
    int newColor[3] = {redVal, grnVal, bluVal};
    setColor(1, newColor);

    delay(wait); // Pause for 'wait' milliseconds before resuming the loop

    if (DEBUG) { // If we want serial output, print it at the 
      if (i == 0 or i % loopCount == 0) { // beginning, and every loopCount times
        Serial.print("Loop/RGB: #");
        Serial.print(i);
        Serial.print(" | ");
        Serial.print(redVal);
        Serial.print(" / ");
        Serial.print(grnVal);
        Serial.print(" / ");  
        Serial.println(bluVal); 
      } 
      DEBUG += 1;
    }
  }
  // Update current values for next loop
  prevR = redVal; 
  prevG = grnVal; 
  prevB = bluVal;
  delay(hold); // Pause for optional 'wait' milliseconds before resuming the loop
}

