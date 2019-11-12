#pragma once

#include <Arduino.h>

#include "char_string.h"
#include "readable.h"
#include "writeable.h"

class EEPROM_16kb : public IReadable, public IWriteable
{
    private:
        const uint16_t i2c_address = 0x0;
        const uint16_t page_size = 128; 

        int get_control_byte() const;
        void write_address(uint16_t address) const;
        void write_page(uint16_t address, const char* data, uint32_t size);
        void read(uint16_t address, char* data, uint32_t size) const;

    public:
        const uint32_t size = page_size * 512ull;

        ~EEPROM_16kb() {}

        void write(unsigned short address, const char* data, unsigned long size) override;
        CharString read(unsigned short address, unsigned long size) const override;
};
