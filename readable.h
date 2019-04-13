#pragma once

class CharString;

class IReadable
{
    public:
        virtual ~IReadable() {};

        virtual CharString read(unsigned short address, unsigned long size) const = 0;
};
