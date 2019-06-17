#pragma once

#include "char_string.h"
#include "readable.h"
#include "writeable.h"

class SerialStream : public IReadable, public IWriteable
{
    public:
        void write(unsigned short address, const char* data, unsigned long size) override;
        CharString read(unsigned short address, unsigned long size) const override;
};
