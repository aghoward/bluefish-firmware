#include "file.h"
#include "size.h"

unsigned int File::size() const
{
    return ::size(name) + ::size(username) + ::size(password);
}

ostream& operator<<(ostream& stream, const File& file)
{
    stream << file.name << file.username << file.password;
    return stream;
}

istream& operator>>(istream& stream, File& file)
{
    stream >> file.name >> file.username >> file.password;
    return stream;
}
