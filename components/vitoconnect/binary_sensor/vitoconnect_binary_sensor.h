#pragma once

#include "esphome/components/binary_sensor/binary_sensor.h"
#include "../vitoconnect_datapoint.h"

namespace esphome {
namespace vitoconnect {

class OPTOLINKBinarySensor : public binary_sensor::BinarySensor, public Datapoint {

  public:
    OPTOLINKBinarySensor();
    ~OPTOLINKBinarySensor();

    void decode(uint8_t* data, uint8_t length, Datapoint* dp = nullptr) override;
    void encode(uint8_t* raw, uint8_t length, void* data) override;
    void encode(uint8_t* raw, uint8_t length, float data);

};

}  // namespace vitoconnect
}  // namespace esphome