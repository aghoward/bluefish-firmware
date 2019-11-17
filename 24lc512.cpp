#include "24lc512.h"

#include <Arduino.h>
#include <Wire.h>

void EEPROM_16kb::write(unsigned short address, const char* data, unsigned long size)
{
    auto max_write_size = 30u;

    for (auto i = 0u; i < size;) {
        auto page_address = address + i;
        auto remaining_bytes = size - i;
        auto bytes_to_write = (remaining_bytes > max_write_size) ? max_write_size : remaining_bytes;
        if (((page_address + bytes_to_write) % page_size) < bytes_to_write)
            bytes_to_write = page_size - (page_address % page_size);
        write_page(page_address, data + i, bytes_to_write);
        i += bytes_to_write;
    }
}

CharString EEPROM_16kb::read(unsigned short address, unsigned long tsize) const
{
    auto result = CharString(tsize);
    auto* out = result.data();

    auto max_read_size = 30u;
    auto bytes_read = 0u;
    while (bytes_read < tsize)
    {
        auto bytes_left = tsize - bytes_read;
        auto to_read = bytes_left > max_read_size ? max_read_size : bytes_left;
        read(address + bytes_read, out + bytes_read, to_read);
        bytes_read += to_read;
    }

    return result;
}

int EEPROM_16kb::get_control_byte() const
{
    return static_cast<int>((0xa0 + ((i2c_address & 0x7) << 1)) >> 1);
}

void EEPROM_16kb::write_address(uint16_t address) const
{
    for (auto i = 0u; i < sizeof(address); i++)
        Wire.write(static_cast<char>((address >> ((sizeof(address) - i - 1)*8)) & 0xFF));
}

void EEPROM_16kb::write_page(uint16_t address, const char* data, uint32_t size)
{
    Wire.beginTransmission(get_control_byte());
    write_address(address);
    Wire.write(data, size);
    Wire.endTransmission();
    delay(30);
}

void EEPROM_16kb::read(uint16_t address, char* data, uint32_t size) const
{
    Wire.beginTransmission(get_control_byte());
    write_address(address);
    Wire.endTransmission();

    Wire.requestFrom(get_control_byte(), size);

    auto i = 0u;
    while (Wire.available())
        data[i++] = static_cast<char>(Wire.read());
}
