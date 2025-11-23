/*
  optolink.cpp - Connect Viessmann heating devices via Optolink to ESPhome

  Copyright (C) 2023  Philipp Danner

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

// #define VITOWIFI_MAX_QUEUE_LENGTH 64

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/uart/uart_component.h"
#include "esphome/components/sensor/sensor.h"
// #include "vitoconnect_DP.h"
#include "vitoconnect_optolink.h"
#include "vitoconnect_optolinkP300.h"
#include "vitoconnect_optolinkKW.h"
#include "vitoconnect_datapoint.h"

using namespace std;

namespace esphome {
namespace vitoconnect {

/**
 * @brief VitoConnect manages the esphome components, their datapoints and optolink to your Viessmann device.
 * 
 */
class VitoConnect : public uart::UARTDevice, public PollingComponent {
  public:

    VitoConnect() : PollingComponent(0) {}
    
    void setup() override;
    void loop() override;
    void update() override;

    void set_protocol(std::string protocol) { this->protocol = protocol; }
    void register_datapoint(Datapoint *datapoint);

    void onData(std::function<void(const uint8_t* data, uint8_t length, Datapoint* dp)> callback);
    void onError(std::function<void(uint8_t, Datapoint*)> callback);

    /**
     * @brief Enqueue a datapoint for writing.
     * 
     * The onData callback will be launched on success.
     * 
     * @tparam D Type of datapoint (inherited from class `Datapoint`)
     * @tparam T Type of the value to be written
     * @param datapoint Datapoint to be read, passed by reference.
     * @param value Value to be written
     * @return true Enqueueing was successful
     * @return false Enqueueing failed (eg. queue full)
     */
    // template<class D, typename T>
    // bool write(D& datapoint, T value);  // NOLINT todo: make it a const ref or pointer?

  protected:

  private:
    Optolink* _optolink;
    std::vector<Datapoint*> _datapoints;
    std::string protocol;
    struct CbArg {
      CbArg(VitoConnect* vw, Datapoint* d, bool write, uint32_t last_update, uint8_t* data = nullptr) :
        v(vw),
        dp(d),
        w(write),
        la(last_update),
        d(data) {}
      VitoConnect* v;
      Datapoint* dp;
      bool w;
      uint32_t la;
      uint8_t* d;
    };
    static void _onData(uint8_t* data, uint8_t len, void* arg);
    static void _onError(uint8_t error, void* arg);

    std::function<void(uint8_t, Datapoint*)> _onErrorCb;
};

}  // namespace vitoconnect
}  // namespace esphome
