#pragma once

#include <RadioLib.h>


class CustomLR1121 : public LR1121 {
  public:
    CustomLR1121(Module *mod) : LR1121(mod) { }

    bool isReceiving() {
      uint16_t irq = getIrqStatus();
      bool detected = ((irq & RADIOLIB_LR11X0_IRQ_SYNC_WORD_HEADER_VALID) || (irq & RADIOLIB_LR11X0_IRQ_PREAMBLE_DETECTED));
      return detected;
    }
};