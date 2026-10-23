#include "vitoconnect_number.h"
#include "../vitoconnect.h"
#include "esphome/core/log.h"

#include <cmath>

namespace esphome {
namespace vitoconnect {

static const char *const TAG = "vitoconnect.number";

void OPTOLINKNumber::decode(uint8_t* data, uint8_t length, Datapoint* dp) {
  if (length < _length) {
    ESP_LOGW(TAG, "0x%04X: received %d bytes, expected %d", _address, length, _length);
    return;
  }

  float raw = NAN;
  if (_length == 1) {
    raw = (float) data[0];
  } else if (_length == 2) {
    int16_t tmp = (int16_t) (data[1] << 8 | data[0]);
    raw = (float) tmp;
  } else if (_length == 4) {
    uint32_t tmp = (uint32_t) data[3] << 24 | (uint32_t) data[2] << 16 | (uint32_t) data[1] << 8 | data[0];
    raw = (float) tmp;
  }
  publish_state(raw * _factor);
}

void OPTOLINKNumber::encode(uint8_t* raw, uint8_t length, void* data) {
  float value = *reinterpret_cast<float*>(data);
  encode(raw, length, value);
}

void OPTOLINKNumber::encode(uint8_t* raw, uint8_t length, float value) {
  // value is already the raw integer value (not scaled)
  if (_length == 1) {
    raw[0] = (uint8_t) lroundf(value);
  } else if (_length == 2) {
    int16_t tmp = (int16_t) lroundf(value);
    raw[0] = tmp & 0xFF;
    raw[1] = (tmp >> 8) & 0xFF;
  } else if (_length == 4) {
    uint32_t tmp = (uint32_t) llroundf(value);
    raw[0] = tmp & 0xFF;
    raw[1] = (tmp >> 8) & 0xFF;
    raw[2] = (tmp >> 16) & 0xFF;
    raw[3] = (tmp >> 24) & 0xFF;
  }
}

void OPTOLINKNumber::control(float value) {
  if (_parent == nullptr) {
    ESP_LOGE(TAG, "0x%04X: no vitoconnect hub assigned", _address);
    return;
  }
  if (std::isnan(value)) {
    ESP_LOGW(TAG, "0x%04X: refusing to write NaN", _address);
    return;
  }

  // Enforce configured limits again (number traits are only checked by the frontend/call)
  const float min_value = this->traits.get_min_value();
  const float max_value = this->traits.get_max_value();
  if (value < min_value || value > max_value) {
    ESP_LOGW(TAG, "0x%04X: value %.2f outside of [%.2f, %.2f], not written", _address, value, min_value, max_value);
    return;
  }

  const float raw_value = roundf(value / _factor);

  // Check that the raw value fits into the datapoint length
  bool fits = false;
  if (_length == 1) {
    fits = raw_value >= 0.0f && raw_value <= 255.0f;
  } else if (_length == 2) {
    fits = raw_value >= -32768.0f && raw_value <= 32767.0f;
  } else if (_length == 4) {
    fits = raw_value >= 0.0f && raw_value <= 4294967295.0f;
  }
  if (!fits) {
    ESP_LOGW(TAG, "0x%04X: raw value %.0f does not fit into %d bytes", _address, raw_value, _length);
    return;
  }

  uint8_t buf[4] = {0};
  encode(buf, _length, raw_value);

  ESP_LOGW(TAG, "Writing 0x%04X: %.2f (raw %.0f, %d bytes)", _address, value, raw_value, _length);
  if (!_parent->write_datapoint(this, buf)) {
    ESP_LOGW(TAG, "0x%04X: write could not be queued", _address);
  }
  // No publish_state here: the hub reads the datapoint back after the write.
}

}  // namespace vitoconnect
}  // namespace esphome
