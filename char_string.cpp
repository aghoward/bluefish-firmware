#include "char_string.h"

#include <stdlib.h>
#include <Arduino.h>
#include <memory.h>
#include <utility.h>

#include "size.h"
#include "stream.h"


CharString::CharString()
    : _data(nullptr),
    _size(0ul)
{
}

CharString::CharString(unsigned int tsize)
    : _data(nullptr),
    _size(0u)
{
    _data = (char*)realloc(_data, tsize);
    _size = tsize;
}

CharString::CharString(unsigned long tsize)
    : CharString(static_cast<unsigned int>(tsize))
{
}

CharString::~CharString()
{
    free(_data);
}


CharString::CharString(const CharString& other)
{
    _size = other._size;

    if (_size == 0u)
    {
        _data = nullptr;
        return;
    }

    _data = (char*)malloc(_size);
    memcpy(_data, other._data, _size);
}

CharString::CharString(CharString&& other)
{
    _data = other._data;
    _size = other._size;
    other._data = nullptr;
    other._size = 0u;
}

CharString& CharString::operator=(const CharString& other)
{
    if (other._size == 0u)
    {
        if (_size != 0u)
            free(_data);
        return *this;
    }

    _data = (char*)realloc(_data, other._size);
    _size = other._size;
    memcpy(_data, other._data, _size);
    return *this;
}

CharString& CharString::operator=(CharString&& other)
{
    auto* tmp_data = _data;
    auto tmp_size = _size; 
    _data = other._data;
    _size = other._size;
    other._data = tmp_data;
    other._size = tmp_size;

    return *this;
}

CharString::CharString(const char* other)
    : _data(nullptr),
    _size(0u)
{
    _size = get_string_size(other);
    _data = (char*)realloc(_data, _size);
    memcpy(_data, other, _size);
}

unsigned int CharString::get_string_size(const char* other) const
{
    auto i = 0u;
    while (other[i++] != 0x0) {}
    return i - 1;
}

char* CharString::data() const
{
    return _data;
}

unsigned int CharString::size() const
{
    return ::size(_size) + _size;
}

bool CharString::operator==(const CharString& other) const
{
    if (other._size != _size)
        return false;

    for (auto i = 0u; i < _size; i++)
    {
        if (_data[i] != other._data[i])
            return false;
    }

    return true;
}

bool CharString::operator!=(const CharString& other) const
{
    return !(*this == other);
}

CharString& CharString::operator+=(const CharString& other)
{
    _data = (char*) realloc(_data, _size + other._size);
    memcpy(_data + _size, other._data, other._size);
    _size += other._size;
    return *this;
}

void CharString::write(unsigned short address, const char* data, unsigned long size)
{
    memcpy(_data + address, data, size);
}

CharString CharString::read(unsigned short address, unsigned long tsize) const
{
    auto result = CharString(static_cast<unsigned int>(tsize));
    memcpy(result._data, _data + address, tsize);
    return result;
}

ostream& operator<<(ostream& stream, const CharString& string)
{
    stream << string._size;
    stream.write(string._data, string._size);
    return stream;
}

istream& operator>>(istream& stream, CharString& string)
{
    string._size = 0u;
    stream >> string._size;

    string._data = (char*) realloc(string._data, string._size);

    stream.read(string._data, string._size);
    return stream;
}
