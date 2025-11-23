#pragma once

#include "esphome/components/number/number.h"
#include "../vitoconnect_datapoint.h"


namespace esphome {
namespace vitoconnect {
    
class OPTOLINKNumber : public number::Number, public Datapoint {

  public:
    OPTOLINKNumber();
    ~OPTOLINKNumber();

    virtual void control(float value) override;

    void decode(uint8_t* data, uint8_t length, Datapoint* dp = nullptr) override;
    void encode(uint8_t* raw, uint8_t length, void* data) override;
    void encode(uint8_t* raw, uint8_t length, float data);
    void encode(uint8_t* raw, uint8_t length) override;

    void setDivRatio(size_t div) { this->_div_ratio = div; }
  private:
    size_t _div_ratio = 1;
};

}  // namespace vitoconnect
}  // namespace esphome