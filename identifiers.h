#pragma once

#include "stream.h"

enum class IdentifierType
{
    File
};

template <IdentifierType T>
class Identifier
{
    public:
        uint16_t value;

        explicit Identifier(uint16_t id)
            : value(id) {}

        Identifier()
            : value(0u) {}
};

typedef Identifier<IdentifierType::File> FileId;

template <IdentifierType T>
ostream& operator<<(ostream& stream, const Identifier<T>& id)
{
    return stream << id.value;
}

template <IdentifierType T>
istream& operator>>(istream& stream, Identifier<T>& id)
{
    return stream >> id.value;
}
