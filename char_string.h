#pragma once

#include <stdlib.h>
#include <type_traits.h>

#include "readable.h"
#include "writeable.h"
#include "stream.h"

class CharString : public IWriteable, public IReadable
{
    protected:
        char* _data;
        unsigned int _size;
        unsigned int get_string_size(const char* other) const;

    public:
        CharString();
        explicit CharString(unsigned int tsize);
        explicit CharString(unsigned long tsize);
        CharString(const CharString&);
        CharString(CharString&& other);

        CharString& operator=(const CharString& other);
        CharString& operator=(CharString&& other);

        CharString(const char* other);

        template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
        explicit CharString(T value)
            : _data(nullptr),
            _size(0u)
        {
            _data = (char*) realloc(_data, sizeof(T));
            _size = sizeof(T);
            for (auto i = 0u; i < sizeof(T); i++)
                _data[i] = (value >> (i * sizeof(char))) & 0xFF;
        }

        ~CharString();

        template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
        static T as_integral(const char* data)
        {
            T out;
            for (auto i = 0u; i < sizeof(T); i++)
                out |= (data[i] << (i * sizeof(char)));
            return out;
        }

        char* data() const;
        unsigned int size() const;

        bool operator==(const CharString&) const;
        bool operator!=(const CharString& other) const;
        CharString& operator+=(const CharString& other);

        void write(unsigned short address, const char* data, unsigned long size) override;
        CharString read(unsigned short address, unsigned long size) const override;
        
        friend ostream& operator<<(ostream& stream, const CharString& string);
        friend istream& operator>>(istream& stream, CharString& string);
};

ostream& operator<<(ostream& stream, const CharString& string);
istream& operator>>(istream& stream, CharString& string);

