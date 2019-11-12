#include "24lc16b.h"

#include <Arduino.h>
#include <Wire.h>

void EEPROM_2kb::write(unsigned short address, const char* data, unsigned long size)
{
    for (auto i = 0u; i < size;)
    {
        auto block_address = (address + i) / block_size;
        auto page_address = (address + i) % block_size;

        auto remaining_block_bytes = block_size - page_address;
        auto endIndex = (size > remaining_block_bytes) ? remaining_block_bytes : size;

        writeBlock(block_address, page_address, data + i, endIndex - i);
        i += remaining_block_bytes;
    }
}

CharString EEPROM_2kb::read(unsigned short address, unsigned long tsize) const
{
    auto result = CharString(tsize);
    auto* out = result.data();

    for (auto i = 0u; i < tsize; out+=i)
    {
        auto block_address = (address + i) / block_size;
        auto page_address = (address + i) % block_size;

        auto bytes_remaining = tsize - i;
        auto block_bytes_remaining = block_size - page_address;
        auto read_size = (bytes_remaining > block_bytes_remaining) ? block_bytes_remaining : bytes_remaining;
        readBlock(block_address, page_address, out, read_size);

        i += read_size;
    }

    return result;
}

int EEPROM_2kb::getBlockAddress(unsigned short block) const
{
    return static_cast<int>(0x50 + (block & 0x7));
}

void EEPROM_2kb::writeBlock(unsigned short block, unsigned short address, const char* data, unsigned long size)
{
    for (auto i = 0u; i < size;) {
        auto page_address = address + i;

        auto page_offset = page_address % page_size;
        auto remaining_page_bytes = page_size - page_offset;

        auto remaining_data_bytes = size - i;
        auto bytes_to_write = (remaining_data_bytes > remaining_page_bytes) ? remaining_page_bytes : remaining_data_bytes;
        writePage(block, page_address, data + i, bytes_to_write);

        i += bytes_to_write;
    }
}

void EEPROM_2kb::writePage(unsigned short block, unsigned short address, const char* data, unsigned long size)
{
    Wire.beginTransmission(getBlockAddress(block));
    Wire.write(static_cast<char>(address & 0xFF));
    for (auto i = 0u; i < size; i++)
        Wire.write(data[i]);
    Wire.endTransmission();
    delay(30);
}

void EEPROM_2kb::readBlock(unsigned short block, unsigned short address, char* data, unsigned long size) const
{
    for (auto i = 0u; i < size; i += page_size)
    {
        auto bytes_remaining = static_cast<unsigned int>(size - i);
        auto read_size = (bytes_remaining > page_size) ? page_size : bytes_remaining;
        readPage(block, address + i, data + i, read_size);
    }
}

void EEPROM_2kb::readPage(unsigned short block, unsigned short address, char* data, unsigned long size) const
{
    Wire.beginTransmission(getBlockAddress(block));
    Wire.write(static_cast<char>(address & 0xFF));
    Wire.endTransmission();

    Wire.requestFrom(getBlockAddress(block), size);
    auto i = 0u;
    while (Wire.available())
        data[i++] = static_cast<char>(Wire.read());
}
