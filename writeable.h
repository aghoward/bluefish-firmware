#pragma once

class IWriteable
{
    public:
        virtual ~IWriteable() {};

        virtual void write(unsigned short address, const char* data, unsigned long size) = 0;
};

