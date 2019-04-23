#pragma once

#include <utility.h>

#include "stream.h"
#include "size.h"

#define INODE_SIZE 64

template <typename T>
struct INode
{
    unsigned int next;
    unsigned int length;
    T data;

    INode(unsigned int n, unsigned int l, T&& d)
        : next(n), length(l), data(std::move(d)) {}

    INode(T&& d)
        : next(0u), length(::size(d)), data(std::move(d)) {} 

    INode(const T& d)
        : next(0u), length(::size(d)), data(d) {}

    INode()
        : next(0u), length(0u), data() {}

    unsigned int size() const
    {
        return sizeof(next) + sizeof(length) + ::size(data);
    }
};

template <>
struct INode<void>
{
    unsigned int next;
    unsigned int length;

    unsigned int size() const
    {
        return sizeof(next) + sizeof(length);
    }
};

static constexpr unsigned int usable_inode_space = INODE_SIZE - sizeof(INode<void>);

template <typename T>
ostream& operator<<(ostream& stream, const INode<T>& inode)
{
    stream << inode.next << inode.length << inode.data;
    return stream;
}

template <typename T>
istream& operator>>(istream& stream, INode<T>& inode)
{
    stream >> inode.next >> inode.length >> inode.data;
    return stream;
}

istream& operator>>(istream& stream, INode<void>& inode);
