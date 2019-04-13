#pragma once

#include "inode.h"
#include "stream.h"

struct FSMasterBlock
{
    unsigned long usage_record;
    unsigned long file_headers;

    FSMasterBlock()
        : usage_record(1ul),
        file_headers(0ul)
    {}
    FSMasterBlock(const FSMasterBlock&) = default;
    FSMasterBlock(FSMasterBlock&&) = default;
    FSMasterBlock& operator=(const FSMasterBlock&) = default;
    FSMasterBlock& operator=(FSMasterBlock&&) = default;

    unsigned int size() const;
};

ostream& operator<<(ostream& stream, const FSMasterBlock& block);
istream& operator>>(istream& stream, FSMasterBlock& block);

using FSMasterINode = INode<FSMasterBlock>;
