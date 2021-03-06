#include "fs_master_block.h"
#include "stream.h"
#include "size.h"

ostream& operator<<(ostream& stream, const FSMasterBlock& block)
{
    stream << block.free_inodes << block.file_headers << block.encryption_iv << block.challenge;
    return stream;
}

istream& operator>>(istream& stream, FSMasterBlock& block)
{
    stream >> block.free_inodes >> block.file_headers >> block.encryption_iv >> block.challenge;
    return stream;
}

uint16_t FSMasterBlock::size() const
{
    return ::size(free_inodes) + ::size(file_headers) + ::size(encryption_iv) + ::size(challenge);
}
