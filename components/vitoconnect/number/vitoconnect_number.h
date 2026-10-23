#pragma once

#include "esphome/components/number/number.h"
#include "../vitoconnect_datapoint.h"

namespace esphome {
namespace vitoconnect {

class VitoConnect;

/**
 * @brief Writable datapoint exposed as ESPHome number.
 *
 * The value is read cyclically like a sensor (value = raw * factor).
 * Setting the number encodes the value (raw = value / factor), writes it
 * via the optolink and reads it back afterwards. The state is never set
 * optimistically; it always reflects what the device reports.
 */
class OPTOLINKNumber : public number::Number, public Datapoint {

  public:
    OPTOLINKNumber() = default;
    ~OPTOLINKNumber() override = default;

    void set_parent(VitoConnect* parent) { this->_parent = parent; }
    void set_factor(float factor) { this->_factor = factor; }

    void decode(uint8_t* data, uint8_t length, Datapoint* dp = nullptr) override;
    void encode(uint8_t* raw, uint8_t length, void* data) override;
    void encode(uint8_t* raw, uint8_t length, float value);

  protected:
    void control(float value) override;

    VitoConnect* _parent{nullptr};
    float _factor{1.0f};
};

}  // namespace vitoconnect
}  // namespace esphome
