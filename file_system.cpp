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


size_t FileSystem::count_free_space() const
{
    auto free_inodes = 0u;
    for (auto i = 0u; i < sizeof(_master_block.usage_record)*8; i++)
        if (!((_master_block.usage_record >> i) & 0x1))
            free_inodes++;

    return free_inodes * usable_inode_space;
}

size_t FileSystem::count_files() const
{
    auto file_count = 0u;
    for (auto i = 0u; i < sizeof(_master_block.file_headers)*8; i++)
        if ((_master_block.file_headers >> i) &0x1)
            file_count++;
    return file_count;
}

void FileSystem::print_usage()
{
    Serial.print("File System: ");
    Serial.print("usage record: ");
    Serial.print(_master_block.usage_record);
    Serial.print(" file_headers: ");
    Serial.println(_master_block.file_headers);
    Serial.flush();
}

void FileSystem::format()
{
    _master_block = FSMasterBlock();
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
    _master_block = inode.data;
}

either<unsigned int, FileSystemError> FileSystem::write(const File& file)
{
    auto file_size = size(file);

    if (file_size > count_free_space())
        return FileSystemError::NotEnoughDiskSpace;

    auto to_write = write_file_to_string(file);
    auto inode_address = request_free_inode();

    set_file_header(inode_address);

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

    auto inode_to_write = INode<CharString>(
        next_address,
        bytes_to_write,
        std::move(data_to_write));
    _ostream << inode_to_write;
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
    unsigned int next_inode = 0u;
    unsigned int inode_length = 0u;
    unsigned int charstring_length = 0u;
    auto filename = CharString();

    _istream.seekg(address);
    _istream >> next_inode >> inode_length >> charstring_length >> filename;

    return filename;
}

unsigned int FileSystem::request_free_inode()
{
    for (auto i = 1u; i < sizeof(_master_block.usage_record)*8; i++)
    {
        if (!((_master_block.usage_record >> i) & 0x1))
        {
            _master_block.usage_record |= (0x1 << i);
            return i * INODE_SIZE;
        }
    }

    return -1;
}

void FileSystem::free_inode(unsigned int address)
{
    auto inode_number = address_to_inode(address);
    if ((_master_block.usage_record >> inode_number) & 0x1)
        _master_block.usage_record ^= (0x1 << inode_number);
    if ((_master_block.file_headers >> inode_number) & 0x1)
        _master_block.file_headers ^= (0x1 << inode_number);
}

void FileSystem::set_file_header(unsigned int address)
{
    auto inode_number = address / INODE_SIZE;
    _master_block.file_headers |= (0x1 << inode_number);
}

either<unsigned int, FileSystemError> FileSystem::get_next_file_header(unsigned int starting_address) const
{
    for (auto starting_inode = starting_address / INODE_SIZE; starting_inode < sizeof(_master_block.file_headers)*8; starting_inode++)
    {
        if ((_master_block.file_headers >> starting_inode) & 0x1)
            return starting_inode * INODE_SIZE;
    }

    return FileSystemError::FileNotFound;
}
