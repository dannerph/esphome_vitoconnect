#pragma once

#include "esphome/components/switch/switch.h"
#include "../vitoconnect_datapoint.h"

namespace esphome {
namespace vitoconnect {

class OPTOLINKSwitch : public switch_::Switch, public Datapoint {

  public:
    OPTOLINKSwitch();
    ~OPTOLINKSwitch();

    virtual void write_state(bool state) override;

    void decode(uint8_t* data, uint8_t length, Datapoint* dp = nullptr) override;
    void encode(uint8_t* raw, uint8_t length, void* data) override;
    void encode(uint8_t* raw, uint8_t length, bool data);
    void encode(uint8_t* raw, uint8_t length) override;

};

}  // namespace vitoconnect
}  // namespace esphome