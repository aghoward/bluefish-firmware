#include "file_system.h"

#include <Arduino.h>
#include <either.h>
#include <utility.h>
#include <memory.h>

#include "char_string.h"
#include "file.h"
#include "inode.h"
#include "stream.h"
#include "size.h"
#include "vector.h"


unsigned int inode_to_address(unsigned int inode_number)
{
    return inode_number * INODE_SIZE;
}

unsigned int address_to_inode(unsigned int address)
{
    return address / INODE_SIZE;
}

size_t FileSystem::count_free_space()
{
    auto free_inodes = _master_block.free_inodes;
    return free_inodes * usable_inode_space;
}

size_t FileSystem::count_files()
{
    return _master_block.file_headers;
}

const FSMasterBlock& FileSystem::get_master_block() const
{
    return _master_block;
}

void FileSystem::format(const CharString& encryption_iv)
{
    auto total_inodes = _eeprom->size / INODE_SIZE;
    for (auto index = 1u; index < total_inodes; index++)
        free_inode(inode_to_address(index));

    _master_block = FSMasterBlock(total_inodes - 1, 0u, encryption_iv);
    write_master_block();
    delay(50);
    sync_usage_record();
}

void FileSystem::write_master_block()
{
    _ostream.seekg(0);
    _ostream << FSMasterINode(_master_block);
}

void FileSystem::sync_usage_record()
{
    _istream.seekg(0);
    auto inode = FSMasterINode();
    _istream >> inode;
    _master_block = std::move(inode.data);
}

either<unsigned int, FileSystemError> FileSystem::write(const File& file)
{
    auto file_size = size(file);

    if (file_size > count_free_space())
        return FileSystemError::NotEnoughDiskSpace;

    auto to_write = write_file_to_string(file);
    auto inode_address = request_free_inode();

    auto max_size = ::size(to_write);
    for (auto i = 0u; i < max_size;)
    {
        auto bytes_remaining = ::size(to_write) - i;
        auto bytes_to_write = (bytes_remaining < usable_inode_space - sizeof(unsigned int))
            ? bytes_remaining
            : usable_inode_space - sizeof(unsigned int);
        auto next_address = (bytes_remaining > bytes_to_write) ? request_free_inode() : 0u;

        write_inode(inode_address, next_address, i, bytes_to_write, to_write);

        inode_address = next_address;
        i += bytes_to_write;
    }

    write_master_block();
    return file_size;
}

CharString FileSystem::write_file_to_string(const File& file) const
{
    auto file_size = size(file);
    auto to_write = CharString(file_size);
    auto writer = ostream(std::addressof(to_write));
    writer << file;
    return to_write;
}

void FileSystem::write_inode(
        unsigned int inode_address,
        unsigned int next_address,
        unsigned int start_index,
        unsigned int bytes_to_write,
        const CharString& to_write)
{
    _ostream.seekg(inode_address);

    auto data_to_write = to_write.read(start_index, bytes_to_write);
    bool is_file_header = start_index == 0u;

    auto inode_to_write = INode<CharString>(
        next_address,
        bytes_to_write,
        is_file_header,
        std::move(data_to_write));

    _ostream << inode_to_write;

    if (is_file_header)
        _master_block.file_headers++;
}

either<File, FileSystemError> FileSystem::read(const CharString& filename)
{
    auto address_result = get_address_of_file(filename);
    return address_result
        .mapFirst([this] (auto&& a) { return this->read_address_to_file(a); });
}

File FileSystem::read_address_to_file(unsigned int address)
{
    auto file_data = read_file_to_string(address);
    auto file_stream = istream(&file_data);
    auto file = File();
    file_stream >> file;
    return file;
}

CharString FileSystem::read_file_to_string(unsigned int address)
{
    CharString out;
    auto data = read_file_inode(address);
    while (data.next != 0)
    {
        out += data.data;
        data = read_file_inode(data.next);
    }

    out += data.data;
    return out;
}

INode<CharString> FileSystem::read_file_inode(unsigned int address)
{
    _istream.seekg(address);
    auto data = INode<CharString>();
    _istream >> data;
    return data;
}


INode<void> FileSystem::read_inode_header(unsigned int address)
{
    _istream.seekg(address);
    auto data = INode<void>();
    _istream >> data;
    return data;
}

either<unsigned int, FileSystemError> FileSystem::remove(const CharString& filename)
{
    return get_address_of_file(filename)
        .mapFirst([&] (auto&& address)
        {
            auto data = read_inode_header(address);
            auto next_address = data.next;
            free_inode(address);

            while (next_address != 0)
            {
                data = read_inode_header(next_address);
                free_inode(next_address);
                next_address = data.next;
            }

            write_master_block();

            return address;
        });
}

vector<CharString> FileSystem::list_files()
{
    auto file_count = static_cast<unsigned int>(count_files());
    auto filenames = vector<CharString>(file_count);
    auto file_address = 0u;
    for (auto i = 0u; i < file_count; i++)
    {
        get_next_file_header(file_address)
            .match(
                [&](auto address) -> void
                {
                    filenames.push_back(this->get_filename_at_address(address));
                    file_address = address + INODE_SIZE;
                },
                [](auto&&) -> void {}
            );
    }

    return filenames;
}

either<unsigned int, FileSystemError> FileSystem::get_address_of_file(const CharString& filename, unsigned int start_address)
{
    return get_next_file_header(start_address)
        .match([&] (auto address) -> either<unsigned int, FileSystemError> {
            if (filename == this->get_filename_at_address(address))
                return address;
            return this->get_address_of_file(filename, address + INODE_SIZE);
        },
        [] (auto error) -> either<unsigned int, FileSystemError> { return error; });
}

CharString FileSystem::get_filename_at_address(unsigned int address)
{
    auto inode_header = INode<void>();
    unsigned int charstring_length = 0u;
    auto filename = CharString();

    _istream.seekg(address);
    _istream >> inode_header >> charstring_length >> filename;

    return filename;
}

unsigned int FileSystem::request_free_inode()
{
    auto inode_count = _eeprom->size / INODE_SIZE;
    for (auto index = 0u; index < inode_count; index++)
    {
        auto address = inode_to_address(index);
        auto inode = read_inode_header(address);
        if (!inode.flags.in_use) {
            inode.flags.in_use = 1u;
            _ostream.seekg(address);
            _ostream << inode;
            _master_block.free_inodes--;
            return address;
        }
    }

    return -1;
}

void FileSystem::free_inode(unsigned int address)
{
    auto header = read_inode_header(address);

    _ostream.seekg(address);
    _ostream << INode<void>();

    _master_block.free_inodes++;
    if (header.flags.is_file_header)
        _master_block.file_headers--;
}

either<unsigned int, FileSystemError> FileSystem::get_next_file_header(unsigned int starting_address)
{
    if (_master_block.file_headers == 0u)
        return FileSystemError::FileNotFound;

    auto inode_count = _eeprom->size / INODE_SIZE;
    for (auto index = address_to_inode(starting_address); index < inode_count; index++)
    {
        auto address = inode_to_address(index);
        auto header = read_inode_header(address);
        if (header.flags.is_file_header)
            return address;
    }

    return FileSystemError::FileNotFound;
}
