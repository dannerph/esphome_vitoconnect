#include "vitoconnect_sensor.h"

namespace esphome {
namespace vitoconnect {

static const char *TAG = "vitoconnect.sensor";

OPTOLINKSensor::OPTOLINKSensor(){
  // empty
}

OPTOLINKSensor::~OPTOLINKSensor() {
  // empty
}

void OPTOLINKSensor::decode(uint8_t* data, uint8_t length, Datapoint* dp) {
  assert(length >= _length);

  if (!dp) dp = this;

  
  if (_length == 1){         // Commonly percentage with factor /2
    publish_state((float) data[0]);
  }
  else if (_length == 2){   // Commonly temperature with factor /10 or /100
    int16_t tmp = 0;
    tmp = data[1] << 8 | data[0];
    float value = tmp / 1.0f;
    publish_state(value);
  }  
  else if (_length == 4){   // Commonly counter with different factors
    uint32_t tmp = 0;
    tmp = data[3] << 24 | data[2] << 16 | data[1] << 8 | data[0];
    float value = tmp / 1.0f;
    publish_state(value);
  }
}

void OPTOLINKSensor::encode(uint8_t* raw, uint8_t length, void* data) {
  float value = *reinterpret_cast<float*>(data);
  encode(raw, length, value);
}

void OPTOLINKSensor::encode(uint8_t* raw, uint8_t length, float data) {
  assert(length >= _length);

  // Commonly temperature with factor /10 or /100
  if (_length == 2){
    int16_t tmp = floor((data) + 0.5);
    raw[1] = tmp >> 8;
    raw[0] = tmp & 0xFF;
  }

  // Commonly counter with different factors
  if (_length == 4){
    uint32_t tmp = floor((data) + 0.5f);
    raw[3] = tmp >> 24;
    raw[2] = tmp >> 16;
    raw[1] = tmp >> 8;
    raw[0] = tmp & 0xFF;
  }
}

}  // namespace vitoconnect
}  // namespace esphome