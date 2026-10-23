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

#include "vitoconnect.h"

namespace esphome {
namespace vitoconnect {

static const char *TAG = "vitoconnect";

void VitoConnect::setup() {

    this->check_uart_settings(4800, 2, uart::UART_CONFIG_PARITY_EVEN, 8);

    ESP_LOGD(TAG, "Starting optolink with protocol: %s", this->protocol.c_str());
    if (this->protocol.compare("P300") == 0) {
        _optolink = new OptolinkP300(this);
    } else if (this->protocol.compare("KW") == 0) {
        _optolink = new OptolinkKW(this);
    } else if (this->protocol.compare("GWG") == 0) {
        _optolink = new OptolinkGWG(this);
    } else {
      ESP_LOGW(TAG, "Unknown protocol.");
    }

    // optimize datapoint list
    _datapoints.shrink_to_fit();

    if (_optolink) {

      // add onData and onError callbacks
      _optolink->onData(&VitoConnect::_onData);
      _optolink->onError(&VitoConnect::_onError);
      
      // set initial state
      _optolink->begin();

    } else {
      ESP_LOGW(TAG, "Not able to initialize VitoConnect");
    }
}

void VitoConnect::register_datapoint(Datapoint *datapoint) {
    ESP_LOGD(TAG, "Adding datapoint with address %x and length %d", datapoint->getAddress(), datapoint->getLength());
    this->_datapoints.push_back(datapoint);
}

void VitoConnect::loop() {
    if (_optolink) _optolink->loop();
}

void VitoConnect::update() {
  // This will be called every "update_interval" milliseconds.
  if (!_optolink) return;
  ESP_LOGD(TAG, "Schedule sensor update");
  
  for (Datapoint* dp : this->_datapoints) {
      CbArg* arg = new CbArg(this, dp);   
      if (_optolink->read(dp->getAddress(), dp->getLength(), reinterpret_cast<void*>(arg))) {
      } else {
          delete arg;
      }
  }
}

bool VitoConnect::write_datapoint(Datapoint* dp, uint8_t* data) {
  if (!_optolink || !dp || !data) {
    ESP_LOGW(TAG, "Write not possible: optolink not initialized");
    return false;
  }
  CbArg* arg = new CbArg(this, dp, true);
  if (!_optolink->write(dp->getAddress(), dp->getLength(), data, reinterpret_cast<void*>(arg))) {
    ESP_LOGW(TAG, "Write to 0x%04X could not be queued (queue full?)", dp->getAddress());
    delete arg;
    return false;
  }
  ESP_LOGI(TAG, "Write to 0x%04X queued (%d bytes)", dp->getAddress(), dp->getLength());
  return true;
}

void VitoConnect::_onData(uint8_t* data, uint8_t len, void* arg) {
  CbArg* cbArg = reinterpret_cast<CbArg*>(arg);
  if (cbArg->write) {
    // Write confirmed: do not interpret the answer as datapoint value,
    // read the datapoint back instead so the state reflects the device.
    Datapoint* dp = cbArg->dp;
    VitoConnect* v = cbArg->v;
    ESP_LOGI(TAG, "Write to 0x%04X successful, reading back", dp->getAddress());
    CbArg* readArg = new CbArg(v, dp);
    if (!v->_optolink->read(dp->getAddress(), dp->getLength(), reinterpret_cast<void*>(readArg))) {
      delete readArg;
    }
    delete cbArg;
    return;
  }
  cbArg->dp->decode(data, len, cbArg->dp);
  delete cbArg;
}

void VitoConnect::_onError(uint8_t error, void* arg) {
  CbArg* cbArg = reinterpret_cast<CbArg*>(arg);
  if (cbArg->write) {
    ESP_LOGW(TAG, "Write to 0x%04X failed, error %d", cbArg->dp->getAddress(), error);
  } else {
    ESP_LOGD(TAG, "Error received: %d", error);
  }
  if (cbArg->v->_onErrorCb) cbArg->v->_onErrorCb(error, cbArg->dp);
  delete cbArg;
}

}  // namespace vitoconnect
}  // namespace esphome
