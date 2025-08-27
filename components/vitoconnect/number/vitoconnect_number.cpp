#include "vitoconnect_number.h"

namespace esphome {
namespace vitoconnect {

static const char *TAG = "vitoconnect.number";

OPTOLINKNumber::OPTOLINKNumber(){}

OPTOLINKNumber::~OPTOLINKNumber() {}

void OPTOLINKNumber::control(float value) {
  if (value < this->traits.get_min_value()) {
    ESP_LOGW(TAG, "control value of number %s below min_value", this->get_name().c_str());
    value = this->traits.get_min_value();
  }
  if (value > this->traits.get_max_value()) {
    ESP_LOGW(TAG, "control value of number %s above max_value", this->get_name().c_str());
    value = this->traits.get_max_value();
  }

  float step = this->traits.get_step();
  if (step > 0.0f) {
    float tmp = std::round(value / step) * step;
    if( tmp != value) {
      ESP_LOGW(TAG, "control value of number %s not matching step %f", this->get_name().c_str(), step);
      value = tmp;
    }
  }

  ESP_LOGD(TAG, "state of number %s to value: %f", this->get_name().c_str(), value);

  this->_last_update = millis();
  publish_state(value);
}

void OPTOLINKNumber::decode(uint8_t* data, uint8_t length, Datapoint* dp) {
  assert(length >= _length);
  float value = 0.0f;

  if (!dp) dp = this;
  
  if (_length == 1){         // Commonly percentage with factor /2
    value = (float) data[0];
  }
  else if (_length == 2){   // Commonly temperature with factor /10 or /100
    int16_t tmp = 0;
    tmp = data[1] << 8 | data[0];
    value = tmp / 1.0f;
    
  }  
  else if (_length == 4){   // Commonly counter with different factors
    uint32_t tmp = 0;
    tmp = data[3] << 24 | data[2] << 16 | data[1] << 8 | data[0];
    value = tmp / 1.0f;
  } else {
    ESP_LOGW(TAG, "Unsupported length %d", _length);
    return;
  }

  ESP_LOGD(TAG, "decode called with data: %f", value);
  value = value / this->_div_ratio;
  ESP_LOGD(TAG, "decode after div_ratio %d: %f", this->_div_ratio, value);

  publish_state(value);
}

void OPTOLINKNumber::encode(uint8_t* raw, uint8_t length) {
  float value = this->state;
  encode(raw, length, value);
}

void OPTOLINKNumber::encode(uint8_t* raw, uint8_t length, void* data) {
  float value = *reinterpret_cast<float*>(data);
  encode(raw, length, value);
}

void OPTOLINKNumber::encode(uint8_t* raw, uint8_t length, float data) {
  assert(length >= _length);
  float value = data * this->_div_ratio;

  ESP_LOGD(TAG, "encode called with data: %f", data);

  if(_length == 1) {
    int8_t tmp = floor((value) + 0.5);
    raw[0] = tmp;
  }

  // Commonly temperature with factor /10 or /100
  if (_length == 2){
    int16_t tmp = floor((value) + 0.5);
    raw[1] = tmp >> 8;
    raw[0] = tmp & 0xFF;
  }

  // Commonly counter with different factors
  if (_length == 4){
    uint32_t tmp = floor((value) + 0.5f);
    raw[3] = tmp >> 24;
    raw[2] = tmp >> 16;
    raw[1] = tmp >> 8;
    raw[0] = tmp & 0xFF;
  }
}

}  // namespace vitoconnect
}  // namespace esphome