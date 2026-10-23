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

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/uart/uart_component.h"
#include "esphome/components/sensor/sensor.h"
// #include "vitoconnect_DP.h"
#include "vitoconnect_optolink.h"
#include "vitoconnect_optolinkP300.h"
#include "vitoconnect_optolinkKW.h"
#include "vitoconnect_optolinkGWG.h"
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
     * @brief Enqueue a raw write for a datapoint.
     *
     * `data` must contain `dp->getLength()` bytes (already encoded, little
     * endian). The bytes are copied, so `data` may go out of scope afterwards.
     * After a successful write (ACK) the datapoint is read back immediately,
     * so its state reflects what the device actually reports.
     *
     * @param dp Datapoint to write (address and length are taken from it)
     * @param data Raw bytes to write
     * @return true Enqueueing was successful
     * @return false Enqueueing failed (eg. queue full or no optolink)
     */
    bool write_datapoint(Datapoint* dp, uint8_t* data);

  protected:

  private:
    Optolink* _optolink{nullptr};
    std::vector<Datapoint*> _datapoints;
    std::string protocol;
    struct CbArg {
      CbArg(VitoConnect* vw, Datapoint* d, bool w = false) :
        v(vw),
        dp(d),
        write(w) {}
      VitoConnect* v;
      Datapoint* dp;
      bool write;
    };
    static void _onData(uint8_t* data, uint8_t len, void* arg);
    static void _onError(uint8_t error, void* arg);

    std::function<void(uint8_t, Datapoint*)> _onErrorCb;
};

}  // namespace vitoconnect
}  // namespace esphome
