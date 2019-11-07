#include "stream.h"

#include "char_string.h"

ostream& ostream::put(char c)
{
    _device->write(_position++, &c, 1);
    return *this;
}

ostream& ostream::write(const char* s, size_t count)
{
    _device->write(_position, s, count);
    _position += count;
    return *this;
}


ostream& ostream::operator<<(uint8_t c)
{
    put(static_cast<char>(c));
    return *this;
}

char istream::get()
{
    auto data = peek();
    _position++;
    return data;
}

char istream::peek() const
{
    return _device->read(_position, 1).data()[0];
}

istream& istream::read(char* out, ios_base::ios_size_t count)
{
    auto data = _device->read(_position, count);
    _position += count;

    auto* ptr = data.data();

    for (auto i = 0u; i < count; i++)
        *out++ = *ptr++;

    return *this;
}

istream& istream::operator>>(uint8_t& c)
{
    c = static_cast<uint8_t>(get());
    return *this;
}
