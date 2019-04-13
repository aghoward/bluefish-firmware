#pragma once

#include <Arduino.h>
#include <type_traits.h>

#include <string.h>

class ostream;
class istream;

#include "readable.h"
#include "writeable.h"

template <typename T>
class ios_base
{
    protected:
        T* _device;
        size_t _position;


    public:
        typedef decltype(_position) ios_size_t;

        ios_base(T* device)
            : _device(device),
              _position(0u)
        {
        }

        ios_size_t tellg() const
        {
            return _position;
        }

        void seekg(ios_size_t position)
        {
            _position = position;
        }
};

class ostream : public ios_base<IWriteable>
{
    public:
        ostream(IWriteable* device)
            : ios_base(device) {}

        ostream& put(char c);
        ostream& write(const char* s, size_t count);

        template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
        ostream& operator<<(T value)
        {
            char data[sizeof(T)];
            for (auto i = 0u; i < sizeof(T); i++)
                data[i] = (value >> (i*8)) & 0xFF;

            write(data, sizeof(T));
            return *this;
        }
};

class istream : public ios_base<IReadable>
{
    public:
        istream(IReadable* device)
            : ios_base(device) {}

        char get();
        char peek() const;
        istream& read(char* out, ios_size_t count);

        template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
        istream& operator>>(T& value)
        {
            value = static_cast<T>(0);
            char data[sizeof(T)];

            read(data, sizeof(T));
            memcpy(&value, data, sizeof(T));

            return *this;
        }
};
