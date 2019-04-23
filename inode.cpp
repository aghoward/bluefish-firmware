#include "inode.h"
#include "stream.h"


istream& operator>>(istream& stream, INode<void>& inode)
{
    stream >> inode.next >> inode.length;
    return stream;
}
