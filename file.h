#pragma once

#include <utility.h>

#include "char_string.h"
#include "stream.h"
#include "size.h"
#include "file.h"

struct File
{
    CharString name;
    CharString username;
    CharString password;

    ~File() {}
    File() : name(), username(), password() {}
    File(CharString&& n, CharString&& u, CharString&& p)
        : name(std::move(n)),
          username(std::move(u)),
          password(std::move(p))
    {
    }

    File(const File&) = delete;
    File(File&&) = default;
    File& operator=(const File&) = delete;
    File& operator=(File&&) = default;

    unsigned int size() const;
};

ostream& operator<<(ostream& stream, const File& file);
istream& operator>>(istream& stream, File& file);
