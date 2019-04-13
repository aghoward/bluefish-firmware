#include "fs_master_block.h"
#include "stream.h"
#include "size.h"

ostream& operator<<(ostream& stream, const FSMasterBlock& block)
{
    stream << block.usage_record << block.file_headers;
    return stream;
}

istream& operator>>(istream& stream, FSMasterBlock& block)
{
    stream >> block.usage_record >> block.file_headers;
    return stream;
}

unsigned int FSMasterBlock::size() const
{
    return ::size(usage_record) + ::size(file_headers);
}
