#include "serial_stream.h"

#include <Arduino.h>

#include "char_string.h"

void SerialStream::write(unsigned short, const char* data, unsigned long size)
{
    Serial.write(data, size);
}

CharString SerialStream::read(unsigned short, unsigned long size) const
{
    auto output = CharString(size);
    Serial.readBytes(output.data(), size);
    return output;
}
