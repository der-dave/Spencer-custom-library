#include <Arduino.h>
#include <CircuitOS.h>
#include <Spencer.h>
#include <PreparedStatement.h>

PreparedStatement* statement = nullptr;
bool synthesizing = false;

// text to speech (TTS) server address and certificate
const char* ttsUrl = "http://192.168.1.35:5500/api/tts?voice=nanotts%3Ade-DE";    // custom address used for TTS
const char* ttsCert = "false";                                                    // "" -> use default CircuitMess certificate; "false" -> dont use a certificate; "xx:xx:xx ..." -> use custom certificate

void speechPlay(TTSError error, CompositeAudioFileSource* source){
  synthesizing = false;
  if(error != TTSError::OK){
    Serial.printf("Text to speech error %d: %s\n", error, TTSStrings[(int) error]);
    delete source;
    delete statement;
    statement = nullptr;
    LEDmatrix.startAnimation(new Animation(new SerialFlashFileAdapter("GIF-angry.gif")), true);

    return;
  }
  LEDmatrix.startAnimation(new Animation(new SerialFlashFileAdapter("GIF-talk.gif")), true);
  Playback.playMP3(source);
  Playback.setPlaybackDoneCallback([](){
    LEDmatrix.startAnimation(new Animation(new SerialFlashFileAdapter("GIF-wink.gif")), true);

  });
  delete statement;
  statement = nullptr;

}

void BTN_press(){
  if (Net.getState() == WL_CONNECTED) {
    LEDmatrix.startAnimation(new Animation(new SerialFlashFileAdapter("GIF-loading0.gif")), true);
    if(synthesizing){
      Serial.println("Another speech synthesis operation is already pending");
    }else{
      synthesizing = true;
      delete statement;
      statement = new PreparedStatement();
      statement->addTTS("Hello world. This is my first program.", ttsUrl, ttsCert);
      statement->prepareWAV(speechPlay);
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


}