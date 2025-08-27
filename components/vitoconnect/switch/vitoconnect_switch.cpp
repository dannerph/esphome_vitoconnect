#include "vitoconnect_switch.h"

namespace esphome {
namespace vitoconnect {

static const char *TAG = "vitoconnect.switch";

OPTOLINKSwitch::OPTOLINKSwitch(){}

OPTOLINKSwitch::~OPTOLINKSwitch() {}

void OPTOLINKSwitch::write_state(bool value) {
  if (value != 0 && value != 1) {
    ESP_LOGE(TAG, "control value of switch %s not 0 or 1", this->get_name().c_str());
  } else {
    ESP_LOGI(TAG, "state of switch %s to value %d", this->get_name().c_str(), value);
    this->_last_update = millis();
    publish_state(value);
  }
}

void OPTOLINKSwitch::decode(uint8_t* data, uint8_t length, Datapoint* dp) {
  assert(length == 1);
  publish_state(data[0] != 0);
}

void OPTOLINKSwitch::encode(uint8_t* raw, uint8_t length) {
  bool value = this->state;
  encode(raw, length, value);
}

void OPTOLINKSwitch::encode(uint8_t* raw, uint8_t length, void* data) {
  bool value = *reinterpret_cast<bool*>(data);
  encode(raw, length, value);
}

void OPTOLINKSwitch::encode(uint8_t* raw, uint8_t length, bool data) {
  assert(length == 1);
  raw[0] = data ? 1 : 0;
}

}  // namespace vitoconnect
}  // namespace esphome