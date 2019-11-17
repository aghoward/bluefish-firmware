#pragma once

#include <utility.h>

#include "stream.h"
#include "size.h"

#define INODE_SIZE 64

struct Flags
{
    uint8_t in_use: 1;
    uint8_t is_file_header: 1;
    uint8_t reserved: 6;

    Flags(uint8_t used, uint8_t file)
        :
        in_use(used),
        is_file_header(file),
        reserved(0u) {}

    Flags() : Flags(0u, 0u) {}
};

template <typename T>
struct INode
{
    uint16_t next;
    Flags flags;
    T data;

    INode(uint16_t n, bool is_file_header, T&& d)
        :
        next(n),
        flags(1u, is_file_header ? 1u : 0u),
        data(std::move(d)) {}

    INode(uint16_t n, bool is_file_header, const T& d)
        :
        next(n),
        flags(1u, is_file_header ? 1u : 0u),
        data(d) {}

    INode()
        : INode(0u, false, {}) {}

    uint16_t size() const
    {
        return sizeof(next) + sizeof(flags) + ::size(data);
    }
};

template <>
struct INode<void>
{
    uint16_t next;
    Flags flags;

    INode() : next(0u), flags() {}

    uint16_t size() const
    {
        return sizeof(next) + sizeof(flags);
    }
};

static constexpr uint16_t usable_inode_space = INODE_SIZE - sizeof(INode<void>);

template <typename T>
ostream& operator<<(ostream& stream, const INode<T>& inode)
{
    stream << inode.next << inode.flags << inode.data;
    return stream;
}

template <typename T>
istream& operator>>(istream& stream, INode<T>& inode)
{
    stream >> inode.next >> inode.flags >> inode.data;
    return stream;
}

ostream& operator<<(ostream& stream, const INode<void>& inode);
istream& operator>>(istream& stream, INode<void>& inode);

ostream& operator<<(ostream& stream, const Flags& flags);
istream& operator>>(istream& stream, Flags& flags);
