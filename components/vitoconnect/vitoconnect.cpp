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
#include "number/vitoconnect_number.h"

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
    _optolink->loop();
}

void VitoConnect::update() {
  // This will be called every "update_interval" milliseconds.
  ESP_LOGD(TAG, "Schedule sensor update");

  // prioritize writes over reads
  bool foundDirty = false;
  for (Datapoint* dp : this->_datapoints) {
    if(dp->getLastUpdate() != 0) {
      foundDirty = true;
      ESP_LOGD(TAG, "Datapoint with address %x was modified and needs to be written.", dp->getAddress());
      
      uint8_t* data = new uint8_t[dp->getLength()];
      dp->encode(data, dp->getLength());

      // write the modified datapoint
      CbArg* writeCbArg = new CbArg(this, dp, true, dp->getLastUpdate());        
      if (!_optolink->write(dp->getAddress(), dp->getLength(), data, reinterpret_cast<void*>(writeCbArg))) {
        delete writeCbArg;
        return;
      }
      
      // read the same datapoint to verify the previous write
      CbArg* readCbArg = new CbArg(this, dp, false, 0, data);
      if (!_optolink->read(dp->getAddress(), dp->getLength(), reinterpret_cast<void*>(readCbArg))) {
        delete readCbArg;
        return;
      }
    }
  }
  
  if(foundDirty) {
    ESP_LOGD(TAG, "Found dirty datapoint(s), skip polling cycle.");
    return;
  }

  for (Datapoint* dp : this->_datapoints) {
      CbArg* arg = new CbArg(this, dp, false, 0);
      if (_optolink->read(dp->getAddress(), dp->getLength(), reinterpret_cast<void*>(arg))) {
      } else {
          delete arg;
      }
  }
}

void VitoConnect::_onData(uint8_t* data, uint8_t len, void* arg) {
  CbArg* cbArg = reinterpret_cast<CbArg*>(arg);

  if (cbArg->dp->getLastUpdate() > 0) {
    if (!cbArg->w && cbArg->d == nullptr) {
      ESP_LOGD(TAG, "Datapoint with address %x is eventually being written, waiting for confirmation.", cbArg->dp->getAddress());
    } else if (cbArg->w) { // this was a write operation
      ESP_LOGD(TAG, "Write operation for datapoint with address %x %s.", 
             cbArg->dp->getAddress(), data[0] == 0x00 ? "has been completed" : "failed");
    } else if (cbArg->d != nullptr) { // cbArg->d is only set if this read is intended to verify a previous write
      ESP_LOGD(TAG, "Verifying received data for datapoint with address %x.", cbArg->dp->getAddress());

      if (len != cbArg->dp->getLength()) {
        ESP_LOGW(TAG, "Expected length of %d was not met for datapoint with address %x.", cbArg->dp->getLength(), cbArg->dp->getAddress());
      } else if (memcmp(data, cbArg->d, len) == 0) {
        ESP_LOGD(TAG, "Previous write operation for datapoint with address %x was successfully verified.", cbArg->dp->getAddress());
        cbArg->dp->clearLastUpdate();
      } else {
        ESP_LOGW(TAG, "Previous write operation for datapoint with address %x failed verification.", cbArg->dp->getAddress());
      }
    }
  } else if (!cbArg->w) {
    cbArg->dp->decode(data, len, cbArg->dp);
  }

  delete cbArg;
}

void VitoConnect::_onError(uint8_t error, void* arg) {
  ESP_LOGD(TAG, "Error received: %d", error);
  CbArg* cbArg = reinterpret_cast<CbArg*>(arg);
  if (cbArg->v->_onErrorCb) cbArg->v->_onErrorCb(error, cbArg->dp);
  delete cbArg;
}

}  // namespace vitoconnect
}  // namespace esphome
