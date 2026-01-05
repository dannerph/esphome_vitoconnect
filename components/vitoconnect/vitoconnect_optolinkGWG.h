/* 
  optolinkGWG.h - Connect Viessmann heating devices via Optolink GWG protocol to ESPhome

  Copyright (c) 2025 Uwe Wendt

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

#include "vitoconnect_optolink.h"

namespace esphome {
namespace vitoconnect {

/**
 * @brief Protocol implementation class for the Optolink (GWG).
 *
 * This class implements the GWG protocol variant of the Viessmann Optolink.
 *
 * Extended addressing concept:
 * - GWG supports only 1-byte physical addresses (0x00..0xFF).
 * - To support multiple read/write "operation types" (virtual/physical/EEPROM/...),
 *   we encode the operation function in the MSB of the datapoint address.
 *
 *   address = (function << 8) | physical_addr
 *
 * - If function == 0x00, behavior is identical to the legacy implementation:
 *   - READ  -> telegram byte 0xCB
 *   - WRITE -> telegram byte 0xC8
 *
 * - If function != 0x00, telegram byte is selected according to the function table.
 *   The write flag is still honored:
 *   - If write flag does not match the function's read/write direction, the queue entry
 *     is discarded and processing continues with the next entry.
 * 
 *      Anfrage 	                  Funktion 	TelegrammByte (Typ)
 *      VIRTUAL READ 	              1 	      C7
 *      VIRTUAL WRITE               02 	      C4
 *      PHYSICAL READ 	            03 	      CB
 *      PHYSICAL WRITE 	            04 	      C8
 *      EEPROM READ 	              05 	      AE
 *      EEPROM WRITE 	              06 	      AD
 *      PHYSICAL XRAM READ 	        49 	      C5
 *      PHYSICAL XRAM WRITE 	      50 	      C3
 *      PHYSICAL PORT READ 	        51 	      6E
 *      PHYSICAL PORT WRITE 	      52 	      6D
 *      PHYSICAL BE READ 	          53 	      9E
 *      PHYSICAL BE WRITE 	        54 	      9D
 *      PHYSICAL KMBUS RAM READ 	  65 	      33
 *      PHYSICAL KMBUS EEPROM READ 	67 	      43
 */
class OptolinkGWG : public Optolink {
 public:
  explicit OptolinkGWG(uart::UARTDevice* uart);

  void begin();
  void loop();

 private:
  enum OptolinkState : uint8_t {
    INIT,
    IDLE,
    SEND,
    RECEIVE,
    UNDEF
  } _state;

  void _init();
  void _idle();
  void _send();
  void _receive();

  // Drain UART RX buffer to remove delayed or stale bytes.
  void _drain_uart_();

  // Drop invalid datapoints from the queue (e.g. unsupported function or direction mismatch).
  // Returns true if a valid datapoint remains at the front of the queue, false if queue is empty.
  bool _drop_invalid_queue_entries_();

  // Generic activity timestamp (connection watchdog)
  uint32_t _lastMillis;

  // Timestamp when the current request was sent (total RX timeout)
  uint32_t _sendMillis;

  // Timestamp of the last received byte (inter-byte timeout)
  uint32_t _lastRxMillis;

  // Timestamp when READY (0x05) was received (diagnostics)
  uint32_t _readyMillis;

  // Indicates whether we are inside a burst sequence (send next requests immediately)
  bool _burstActive;

  bool _write;

  // Receive buffer for protocol responses
  uint8_t _rcvBuffer[MAX_DP_LENGTH];
  size_t _rcvBufferLen;
  size_t _rcvLen;
};

}  // namespace vitoconnect
}  // namespace esphome
