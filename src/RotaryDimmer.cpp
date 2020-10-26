#include <Rotary.h>
#include <ESP8266WiFi.h>          //ESP8266 Core WiFi Library (you most likely already have this in your sketch)
#include <DNSServer.h>            //Local DNS Server used for redirecting all requests to the configuration portal

#include "wifi_manager.h"

#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <AsyncJson.h>


#include "LittleFS.h" // LittleFS is declared

const uint8_t pin_a = 0;
const uint8_t pin_b = 4;
const uint8_t pin_btn = 5;
const uint8_t pin_pwm = 14;
//const uint8_t pin_pwm = LED_BUILTIN;

class RotaryReader
{
private:
  Rotary r;
  int value;
  int max_value;
public:  
  RotaryReader(int max_value):
    value(0),
    max_value(max_value)
  {}
  
  bool process()
  {
    bool a = digitalRead(pin_a);
    bool b = digitalRead(pin_b);
    bool btn = ! digitalRead(pin_btn); 
    
    switch (r.process(a, b, btn))
    {
      case RotaryOutput::NONE:
        return false;
        break;
      case RotaryOutput::CW:
        value++;
        // Serial.println("CW: ");
        break;
      case RotaryOutput::CCW:
        value--;
        // Serial.println("CCW: ");
        break;
      case RotaryOutput::BTN_DOWN:
        // Serial.println("BTN_DOWN");
        //delay(20);
        break;
      case RotaryOutput::BTN_SHORT:
        // Serial.println("BTN_SHORT");
        if (value > 0)
          value = 0;
        else
          value = 1;
        //delay(20);
        break;
      case RotaryOutput::BTN_LONG:
        // Serial.println("BTN_LONG");
        if (value > 0)
          value = 0;
        else
          value = max_value;
        //delay(20);
        break;
    }
    if (value <= 0)
      value = 0;
    if (value >= max_value)
      value = max_value;
    return true;
  }
  int get()
  {
    return value;
  }
  void set(int new_value)
  {
    value = new_value;
  }
};

const int range = 1024;
RotaryReader r(10);
AsyncWebServer server(80);

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

String getStateJson(){
  StaticJsonDocument<200> doc;
  doc["intensity"] = r.get();
  String buf;
  serializeJsonPretty(doc, buf);
  return buf;
}

void getValues(AsyncWebServerRequest *request) {
  request->send(200, "text/json", getStateJson());
}

//void setValues(AsyncWebServerRequest *request, JsonVariant &json) 
//void setValues(AsyncWebServerRequest *request) 
void setValues(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
{
  StaticJsonDocument<200> doc;
  deserializeJson(doc, data, len);
  int intensity = doc["intensity"].as<int>();
  r.set(intensity);
  Serial.printf("Set from wifi: %d\n", intensity);
  request->send(200, "text/json",  getStateJson());
}

// Define routing
void restServerRouting() {
  server.on("/values", HTTP_GET, getValues);
  server.on("/setvalues", HTTP_PATCH, [](AsyncWebServerRequest *request){
    //nothing and dont remove it
  }, NULL, setValues);
  server.serveStatic("/", LittleFS, "/www/").setDefaultFile("index.html");
  //server.addHandler(jsonHandler);
  server.onNotFound(notFound);
}

ICACHE_RAM_ATTR
void pin_change(){
  if (r.process())
    Serial.printf("Set from rotary: %d\n", r.get());
}

void setup() {
  pinMode(pin_pwm, OUTPUT);
  analogWriteRange(range);
  analogWriteFreq(800);
  analogWrite(pin_pwm, 0); 

  
  pinMode(pin_a, INPUT_PULLUP); 
  pinMode(pin_b, INPUT_PULLUP);
  pinMode(pin_btn, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(pin_a), pin_change, CHANGE);
  attachInterrupt(digitalPinToInterrupt(pin_b), pin_change, CHANGE);
  attachInterrupt(digitalPinToInterrupt(pin_btn), pin_change, CHANGE);

  Serial.begin(115200);

  setup_wifi("lamp");
  
  LittleFS.begin(); // Before routing!
  
  restServerRouting();
  server.begin();
  
  Serial.println("");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

template <uint32_t iir_factor>
class IIR{
  int32_t buffer;
  public:
  IIR(): buffer(0){};

  int32_t update(int32_t target)
  {
    target = target << 8;
    buffer = (((buffer * (iir_factor - 1)) + target)) / iir_factor;
    if (buffer < target)
      buffer += (iir_factor-1);
    return  buffer >> 8;
  }
};

void loop() {
  static IIR<16> filter;
  uint32_t target = (1 << r.get()) -1; // 2^n-1
  uint32_t output = filter.update(target);
  analogWrite(pin_pwm, output); 
  delay(10);
}
