#pragma once

#include "char_string.h"
#include "inode.h"
#include "stream.h"
#include <utility.h>

struct FSMasterBlock
{
    uint32_t free_inodes;
    uint32_t file_headers;
    CharString encryption_iv;
    CharString challenge;

    FSMasterBlock(uint32_t free, uint32_t files, const CharString& iv, const CharString& _challenge)
        :
        free_inodes(free),
        file_headers(files),
        encryption_iv(iv),
        challenge(_challenge)
    {}

    FSMasterBlock()
        : FSMasterBlock(0u, 0u, {}, {})
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
