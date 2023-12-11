/* VitoWiFi

Copyright 2019 Bert Melis

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

/**
 * @file OptolinkKW.h
 * @brief Optolink API definions (protocol KW).
 *
 * This file contains all method definitions for the Optolink for the 
 * protocol KW.
 */

#pragma once

#include "vitoconnect_optolink.h"
// #include <Arduino.h>  // for millis

namespace esphome {
namespace vitoconnect {

/**
 * @brief Protocol implementation class for the Optolink (KW).
 * 
 * This class is a KW version of the Optolink.
 */
class OptolinkKW : public Optolink {
 public:
  /**
   * @brief Construct the Optolink object (KW)
   * 
   * @param uart UARTDevice object to be used. Pass by reference.
   */
  explicit OptolinkKW(uart::UARTDevice* uart);

  /**
   * @brief Method to start the Optolink.
   * 
   * Calling this method starts the UARTDevice (passed in the constructor)
   * with the right settings.
   */
  void begin();

  /**
   * @brief Method to keep the Optolink running.
   * 
   * This method has to be called frequently. The Optolink object works by 
   * polling the UARTDevice. If applicable, add a delay between calls to 
   * feed a watchdog (needed for ESP8266 and ESP32).
   */
  void loop();

 private:
  enum OptolinkState : uint8_t {
    INIT,
    IDLE,
    SYNC,
    SEND,
    RECEIVE,
    UNDEF
  } _state;
  void _init();
  void _idle();
  void _sync();
  void _send();
  void _receive();
  uint32_t _lastMillis;
  bool _write;
  uint8_t _rcvBuffer[MAX_DP_LENGTH];
  size_t _rcvBufferLen;
  size_t _rcvLen;
};

}  // namespace vitoconnect
}  // namespace esphome
