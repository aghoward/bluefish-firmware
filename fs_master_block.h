#pragma once

#include "inode.h"
#include "stream.h"

struct FSMasterBlock
{
    uint32_t free_inodes;
    uint32_t file_headers;

    FSMasterBlock(uint32_t free, uint32_t files)
        : free_inodes(free), file_headers(files) {}

    FSMasterBlock()
        : FSMasterBlock(0u, 0u)
    {}
    FSMasterBlock(const FSMasterBlock&) = default;
    FSMasterBlock(FSMasterBlock&&) = default;
    FSMasterBlock& operator=(const FSMasterBlock&) = default;
    FSMasterBlock& operator=(FSMasterBlock&&) = default;

    uint16_t size() const;
};

ostream& operator<<(ostream& stream, const FSMasterBlock& block);
istream& operator>>(istream& stream, FSMasterBlock& block);

using FSMasterINode = INode<FSMasterBlock>;
