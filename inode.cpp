#include <Arduino.h>

#include "inode.h"
#include "stream.h"

ostream& operator<<(ostream& stream, const INode<void>& inode)
{
    return stream << inode.next << inode.flags;
}

istream& operator>>(istream& stream, INode<void>& inode)
{
    return stream >> inode.next >> inode.flags;
}

ostream& operator<<(ostream& stream, const Flags& flags)
{
    uint8_t data = 0u;
    memcpy(&data, &flags, sizeof(data));
    stream << data;
    return stream;
}

istream& operator>>(istream& stream, Flags& flags)
{
    uint8_t data = 0u;
    stream >> data;
    memcpy(&flags, &data, sizeof(data));
    return stream;
}
