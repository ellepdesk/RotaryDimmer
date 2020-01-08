#include <Rotary.h>

const uint8_t pin_a = 0;
const uint8_t pin_b = 4;
const uint8_t pin_btn = 5;
const uint8_t pin_pwm = 14;


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
        Serial.println("CW: ");
        break;
      case RotaryOutput::CCW:
        value--;
        Serial.println("CCW: ");
        break;
      case RotaryOutput::BTN_DOWN:
        Serial.println("BTN_DOWN");
        delay(20);
        break;
      case RotaryOutput::BTN_SHORT:
        Serial.println("BTN_SHORT");
        if (value > 0)
          value = 0;
        else
          value = 1;
        delay(20);
        break;
      case RotaryOutput::BTN_LONG:
        Serial.println("BTN_LONG");
        if (value > 0)
          value = 0;
        else
          value = max_value;
        delay(20);
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
};

void empty_isr()
{}

const int range = 1024;
RotaryReader r(10);

void setup() {
  pinMode(pin_a, INPUT_PULLUP); 
  pinMode(pin_b, INPUT_PULLUP);
  pinMode(pin_btn, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(pin_a), empty_isr, CHANGE);
  attachInterrupt(digitalPinToInterrupt(pin_b), empty_isr, CHANGE);
  attachInterrupt(digitalPinToInterrupt(pin_btn), empty_isr, CHANGE);
  
  Serial.begin(115200);
  Serial.println("Hello");
  pinMode(pin_pwm, OUTPUT);
  analogWriteRange(range);
  analogWriteFreq(800);
}


void loop() {
  const int iir_factor = 128;
  int output, setpoint = 0;
  if (r.process())
  {
    int value = r.get();
    Serial.print(value);
    analogWrite(pin_pwm, output);
    setpoint = (1 << value) -1; // 2^n-1
  }
  output = ((output * iir_factor - 1) + setpoint) / iir_factor;
  //Serial.print(" ");
  //Serial.println(output);
  delay(100);
}
