/*
 * Example for how to use SinricPro TV device:
 * - setup a TV device
 * - handle request using callbacks
 *
 * If you encounter any issues:
 * - check the readme.md at https://github.com/sinricpro/esp8266-esp32-sdk/blob/master/README.md
 * - ensure all dependent libraries are installed
 *   - see https://github.com/sinricpro/esp8266-esp32-sdk/blob/master/README.md#arduinoide
 *   - see https://github.com/sinricpro/esp8266-esp32-sdk/blob/master/README.md#dependencies
 * - open serial monitor and check whats happening
 * - check full user documentation at https://sinricpro.github.io/esp8266-esp32-sdk
 * - visit https://github.com/sinricpro/esp8266-esp32-sdk/issues and check for existing issues or open a new one
 */

#include <Arduino.h>
#ifdef ESP8266 
       #include <ESP8266WiFi.h>
#endif 
#ifdef ESP32   
       #include <WiFi.h>
#endif

#include "SinricPro.h"
#include "SinricProTV.h"
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>
uint16_t RECV_PIN = 14; //Pin you wish receive IR on
IRrecv irrecv(RECV_PIN);

decode_results results;

#include "Servo.h"

Servo myServo;


const uint16_t kIrLed = 2 ; //Pin you wish to send IR on
IRsend irsend(kIrLed);

#define WIFI_SSID         "Wu-Tag Lan"    //WIFI Name
#define WIFI_PASS         "ronitchangedthepassword"    // WIFI Password
#define APP_KEY           "7182063a-1719-4fc2-86cf-7fdb2d4701fd"      //Sinric App Key, should look like "de0bxxxx-1x3x-4x3x-ax2x-5dabxxxxxxxx"
#define APP_SECRET        "c251fc11-c39c-4505-8a15-4c5ac7d16fba-a2d3c5c1-b47e-42ce-9f09-c9ae6911b3e9"   // Sinric App Secret, should look like "5f36xxxx-x3x7-4x3x-xexe-e86724a9xxxx-4c4axxxx-3x3x-x5xe-x9x3-333d65xxxxxx"
#define TV_ID             "6503c69b139df6c2e7eb59e7"    // Sinric TV Device ID Should look like "5dc1564130xxxxxxxxxxxxxx"
#define TV_ID_2           "xxxxxxxxxxxxxxxxxxxxxxxx"    // Sinric TV Device ID Should look like "5dc1564130xxxxxxxxxxxxxx"
#define BAUD_RATE         9600

bool tvPowerState;
unsigned int tvVolume;
unsigned int tvChannel;
bool tvMuted;
bool clockPowerState;

// channelNames used to convert channelNumber into channelName
// please put in your TV channel names
// channel numbers starts counting from 0 (Netflix is 0 in example below)!
// For channel names, this can be trial and error as amazon are yet to explicitedly publish the supported channels. The following is a list supported on Fire Tv, i would imagine that these should work: https://developer.amazon.com/docs/video-skills-fire-tv-apps/channels-list.html 
const char* channelNames[] = {
  "netflix vic", //(Alexa only reconises Netflix written like this, i believe its a Virgin Media thing. However saying "Change channel to Netflix on TV" works fine)
  "BBC 1", 
  "BBC 2", 
  "ITV", 
  "channel 4", 
  "dave", 
  "bbc four", 
  "", 
  "rtl", 
  "RTL II", 
  "SUPER RTL", 
  "KiKA"
};

#define MAX_CHANNELS sizeof(channelNames) / sizeof(channelNames[0])  // just to determine how many channels are in channelNames array

// map channelNumbers used to convert channelName into channelNumber
// This map is initialized in "setupChannelNumbers()" function by using the "channelNames" array
std::map<String, unsigned int> channelNumbers;

void setupChannelNumbers() {
  for (unsigned int i=0; i < MAX_CHANNELS; i++) {
    channelNumbers[channelNames[i]] = i;
  }
}

// TV device callbacks

bool onAdjustVolume(const String &deviceId, int &volumeDelta, bool volumeDefault) {
  if (volumeDelta > 0) {irsend.sendNEC(0xFD22DD, 32);irsend.sendNEC(0xFD22DD, 32);irsend.sendNEC(0xFD22DD, 32);irsend.sendNEC(0xFD22DD, 32);irsend.sendNEC(0xFD22DD, 32);} //Insert IR for Volume Up here.
  if (volumeDelta < 0) {irsend.sendNEC(0xFDC23D, 32);irsend.sendNEC(0xFDC23D, 32);irsend.sendNEC(0xFDC23D, 32);irsend.sendNEC(0xFDC23D, 32);irsend.sendNEC(0xFDC23D, 32);} //Insert IR for Volume Down here.
  tvVolume += volumeDelta;  // calcualte new absolute volume
  Serial.printf("Volume changed about %i to %i\r\n", volumeDelta, tvVolume);
  volumeDelta = tvVolume; // return new absolute volume
  return true;
}

bool onChangeChannel(const String &deviceId, String &channel) {
  tvChannel = channelNumbers[channel]; // save new channelNumber in tvChannel variable
  Serial.printf("Change channel blah to \"%s\" (channel number %d)\r\n", channel.c_str(), tvChannel);
  if (channel == "BBC 2") {irsend.sendNEC(0XFD807F, 32); delay (250); irsend.sendNEC(0xFD00FF, 32); delay (250); irsend.sendNEC(0xFD40BF, 32);} //Change IR sequence inside of {} for each channel.
  if (channel == "BBC 1") {irsend.sendNEC(0XFD807F, 32); delay (250); irsend.sendNEC(0xFD00FF, 32); delay (250); irsend.sendNEC(0XFD807F, 32);}
  if (channel == "netflix vic") {irsend.sendNEC(0xFDE21D, 32); delay (10000); irsend.sendNEC(0xFDA857, 32); delay (500); irsend.sendNEC(0xFD15EA, 32);}
 
  
  return true;
}

bool onChangeChannelNumber(const String& deviceId, int channelNumber, String& channelName) {
  tvChannel = channelNumber; // update tvChannel to new channel number
  
  if (tvChannel < 0) tvChannel = 0;
  if (tvChannel > MAX_CHANNELS-1) tvChannel = MAX_CHANNELS-1;
  
  
  delay(250);
  

  channelName = channelNames[tvChannel]; // return the channelName
  

  Serial.printf("Change to channel to %d (channel name \"%s\")\r\n", tvChannel, channelName.c_str());
   
  
  
  return true;
}

bool onMediaControl(const String &deviceId, String &control) {
  Serial.printf("MediaControl: %s\r\n", control.c_str()); //Change and/or insert IR code inbetween {}
  if (control == "Play") {irsend.sendNEC(0xFD15EA, 32);}           
  if (control == "Pause") {irsend.sendNEC(0xFD956A, 32);}          
  if (control == "Stop") {irsend.sendNEC(0xFD5AA5, 32);}           
  if (control == "StartOver") {irsend.sendNEC(0xFD5AA5, 32); irsend.sendNEC(0xFD15EA, 32);}      
  if (control == "Previous") {}      
  if (control == "Next") {}           
  if (control == "Rewind") {irsend.sendNEC(0xFD1AE5, 32);}         
  if (control == "FastForward") {irsend.sendNEC(0xFD9A65, 32);}    
  return true;
}

bool onMute(const String &deviceId, bool &mute) {
  if(deviceId=="TV_ID"){
  
  Serial.printf("TV volume is %s\r\n", mute?"muted":"unmuted");
  tvMuted = mute; // set tvMuted state
  if(mute){
      
        irsend.sendNEC(0xFD18E7, 32); //Insert Mute button IR for Mute 
      }
       else{
        irsend.sendNEC(0xFD708F, 32); //Insert Mute button IR for unmute (Normally the same as above)
        }
  }
  else if(deviceId==TV_ID_2){
    Serial.printf("TV volume is %s\r\n", mute?"muted":"unmuted");
  tvMuted = mute; // set tvMuted state
  if(mute){
      
        irsend.sendSony(0x140A, 15); //Insert Mute button IR for Mute 
      }
       else{
        irsend.sendSony(0x140A, 15); //Insert Mute button IR for unmute (Normally the same as above)
        }
        }      
  
  return true; 
}

bool onPowerState(const String &deviceId, bool &state) {
  irrecv.enableIRIn();

  myServo.attach(D4);
  myServo.write(0);
  

  if(deviceId==TV_ID){
    Serial.printf("Stair Lights turned %s\r\n", state?"on":"off");
  tvPowerState = state;
  if(state){
    for (int pos = 180; pos >= 0; pos -= 1) { // goes from 180 degrees to 0 degrees
    myServo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(10);                       // waits 15ms for the servo to reach the position
  }
        for (int pos = 0; pos <= 180; pos += 1) { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    myServo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(10);                       // waits 15ms for the servo to reach the position
  }
  
  
      }
       else{
        irsend.sendNEC(0xFF827D, 32, 2); //Power Off IR
        }// set powerState
  }
    else if(deviceId==TV_ID_2){
     Serial.printf("TV turned %s\r\n", state?"on":"off");
  tvPowerState = state;
  if(state){
      
        irsend.sendNEC(0xFDB04F, 32, 3); //Power On IR
      }
       else{
        irsend.sendNEC(0xFDB04F, 32, 3); //Power Off IR
        }// set powerState 
  }
  return true; 
}

bool onSelectInput(const String &deviceId, String &input) {

  Serial.printf("Input changed to %s\r\n", input.c_str());
   if (input = "HDMI 1") {irsend.sendNEC(0xFD48B7, 32); delay (250); irsend.sendNEC(0xFD18E7, 32); delay (250); irsend.sendNEC(0xFDA857, 32);} //Set your IR sequence to change Input on TV
  
  return true;
}

bool onSetVolume(const String &deviceId, int &volume) {
  Serial.printf("Volume set to:  %i\r\n", volume);
  tvVolume = volume; // update tvVolume
  return true;
}

bool onSkipChannels(const String &deviceId, const int channelCount, String &channelName) {
  tvChannel += channelCount; // calculate new channel number
  if (tvChannel < 0) tvChannel = 0;
  if (tvChannel > MAX_CHANNELS-1) tvChannel = MAX_CHANNELS-1;
  channelName = String(channelNames[tvChannel]); // return channel name

  Serial.printf("Skip channel: %i (number: %i / name: \"%s\")\r\n", channelCount, tvChannel, channelName.c_str());

  return true;
}

// setup function for WiFi connection
void setupWiFi() {
  irsend.begin();
  
  Serial.printf("\r\n[Wifi]: Connecting");
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.printf(".");
    delay(250);
  }
  IPAddress localIP = WiFi.localIP();
  Serial.printf("connected!\r\n[WiFi]: IP-Address is %d.%d.%d.%d\r\n", localIP[0], localIP[1], localIP[2], localIP[3]);
  irrecv.enableIRIn();
}

void dump(decode_results *results) {
  // Dumps out the decode_results structure.
  // Call this after IRrecv::decode()
  uint16_t count = results->rawlen;
  if (results->decode_type == UNKNOWN) {
    Serial.print("Unknown encoding: ");
  } else if (results->decode_type == NEC) {
    Serial.print("Decoded NEC: ");
  } else if (results->decode_type == SONY) {
    Serial.print("Decoded SONY: ");
  } else if (results->decode_type == RC5) {
    Serial.print("Decoded RC5: ");
  } else if (results->decode_type == RC5X) {
    Serial.print("Decoded RC5X: ");
  } else if (results->decode_type == RC6) {
    Serial.print("Decoded RC6: ");
  } else if (results->decode_type == RCMM) {
    Serial.print("Decoded RCMM: ");
  } else if (results->decode_type == PANASONIC) {
    Serial.print("Decoded PANASONIC - Address: ");
    Serial.print(results->address, HEX);
    Serial.print(" Value: ");
  } else if (results->decode_type == LG) {
    Serial.print("Decoded LG: ");
  } else if (results->decode_type == JVC) {
    Serial.print("Decoded JVC: ");
  } else if (results->decode_type == AIWA_RC_T501) {
    Serial.print("Decoded AIWA RC T501: ");
  } else if (results->decode_type == WHYNTER) {
    Serial.print("Decoded Whynter: ");
  } else if (results->decode_type == NIKAI) {
    Serial.print("Decoded Nikai: ");
  }
  serialPrintUint64(results->value, 16);
  Serial.print(" (");
  Serial.print(results->bits, DEC);
  Serial.println(" bits)");
  Serial.print("Raw (");
  Serial.print(count, DEC);
  Serial.print("): {");

  for (uint16_t i = 1; i < count; i++) {
    if (i % 100 == 0)
      yield();  // Preemptive yield every 100th entry to feed the WDT.
    if (i & 1) {
      Serial.print(results->rawbuf[i] * kRawTick, DEC);
    } else {
      Serial.print(", ");
      Serial.print((uint32_t) results->rawbuf[i] * kRawTick, DEC);
    }
  }
  Serial.println("};");
}

// setup function for SinricPro
void setupSinricPro() {
  // add device to SinricPro
  SinricProTV& myTV = SinricPro[TV_ID];
  SinricProTV& myTV2 = SinricPro[TV_ID_2]; //add second device

  // set callback functions to device
  myTV.onAdjustVolume(onAdjustVolume);
  myTV.onChangeChannel(onChangeChannel);
  myTV.onChangeChannelNumber(onChangeChannelNumber);
  myTV.onMediaControl(onMediaControl);
  myTV.onMute(onMute);
  myTV.onPowerState(onPowerState);
  myTV.onSelectInput(onSelectInput);
  myTV.onSetVolume(onSetVolume);
  myTV.onSkipChannels(onSkipChannels);

  myTV2.onPowerState(onPowerState);  //Specify what Second Device Functions you'd like
  myTV2.onMute(onMute);
  myTV2.onAdjustVolume(onAdjustVolume);


  // setup SinricPro
  SinricPro.onConnected([](){ Serial.printf("Connected to SinricPro\r\n"); }); 
  SinricPro.onDisconnected([](){ Serial.printf("Disconnected from SinricPro\r\n"); });
  SinricPro.begin(APP_KEY, APP_SECRET);
}

// main setup function
void setup() {
  Serial.begin(BAUD_RATE); Serial.printf("\r\n\r\n");
  Serial.printf("%d channels configured\r\n", MAX_CHANNELS);

irrecv.enableIRIn();

  setupWiFi();
  setupSinricPro();
  setupChannelNumbers();
}

void loop() {
  SinricPro.handle();
    if (irrecv.decode(&results)) {
    dump(&results);
    Serial.println("DEPRECATED: Please use IRrecvDumpV2.ino instead!");
    irrecv.resume();  // Receive the next value
  }
}
