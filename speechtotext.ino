#include <Arduino.h>
#include <CircuitOS.h>
#include <Spencer.h>

const char* listenResult;
IntentResult* intentResult;
bool processingIntent = false;
bool detectingIntent = false;

// speech to text (STT) server address and certificate
const char* sttUrl = "http://xxx.xxx.xxx.xxx:xxxxx/api/speech-to-intent";    // custom address used for STT
const char* sttCert = "false";                                               // "" -> use default CircuitMess certificate; "false" -> dont use a certificate; "xx:xx:xx ..." -> use custom certificate

void listenProcess(){
}

void listenError(){
}

void listenCheck(){
  if(listenResult != nullptr && !processingIntent){
    processingIntent = true;
    delete intentResult;
    intentResult = nullptr;
    SpeechToIntent.addJob({ listenResult, &intentResult, sttUrl, sttCert });

    LEDmatrix.startAnimation(new Animation(new SerialFlashFileAdapter("GIF-loading2.gif")), true);

  }
  if(processingIntent && intentResult != nullptr){
    detectingIntent = false;
    processingIntent = false;
    listenResult = nullptr;
    if(intentResult->error != IntentResult::Error::OK && intentResult->error != IntentResult::Error::INTENT){
      Serial.printf("Speech to text error %d: %s\n", intentResult->error, STIStrings[(int) intentResult->error]);
      listenError();
      delete intentResult;
      intentResult = nullptr;
      return;
    }
    if(intentResult->intent == nullptr){
      intentResult->intent = (char*) malloc(5);
      memcpy(intentResult->intent, "NONE", 5);
    }
    if(intentResult->transcript == nullptr){
      intentResult->transcript = (char*) malloc(1);
      memset(intentResult->transcript, 0, 1);
    }
    listenProcess();
    delete intentResult;
    intentResult = nullptr;
  }
}

void BTN_press(){
  if (Net.getState() == WL_CONNECTED) {
    LEDmatrix.startAnimation(new Animation(new SerialFlashFileAdapter("GIF-listen.gif")), true);
    if(detectingIntent){
      Serial.println("Another listen and intent detection operation is already pending");
    }else{
      detectingIntent = true;
      listenResult = nullptr;
      Recording.addJob({ &listenResult });
    }
  }
}


void setup() {
  Serial.begin(115200);
  Serial.println();
  Spencer.begin();
  Spencer.loadSettings();
  Input::getInstance()->setBtnPressCallback(BTN_PIN, BTN_press);

  LEDmatrix.startAnimation(new Animation(new SerialFlashFileAdapter("GIF-wifi.gif")), true);
  Net.connect([](wl_status_t state){
    if (Net.getState() == WL_CONNECTED) {
      Playback.playMP3(SampleStore::load(SampleGroup::Special, "badum0"));
      LEDmatrix.startAnimation(new Animation(new SerialFlashFileAdapter("GIF-smile.gif")), true);
    } else {
      Playback.playMP3(SampleStore::load(SampleGroup::Error, "wifi"));
      LEDmatrix.startAnimation(new Animation(new SerialFlashFileAdapter("GIF-noWifi.gif")), true);
    }

  });
}

void loop() {
  LoopManager::loop();
  listenCheck();
}