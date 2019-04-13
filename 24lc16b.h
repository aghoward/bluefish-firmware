#pragma once

#include <Arduino.h>

#include "char_string.h"
#include "readable.h"
#include "writeable.h"

class EEPROM : public IReadable, public IWriteable
{
    private:
        const unsigned short i2c_address = 0x50;
        const unsigned short block_size = 256;
        const unsigned short page_size = 16;

        int getBlockAddress(unsigned short block) const;
        void writeBlock(unsigned short block, unsigned short address, const char* data, unsigned long size);
        void writePage(unsigned short block, unsigned short address, const char* data, unsigned long size);
        void readBlock(unsigned short block, unsigned short address, char* data, unsigned long size) const;
        void readPage(unsigned short block, unsigned short address, char* data, unsigned long size) const;

    public:
        const size_t size = block_size * 8;

        ~EEPROM() {}

        void write(unsigned short address, const char* data, unsigned long size) override;
        CharString read(unsigned short address, unsigned long size) const override;
};
