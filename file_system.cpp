#include "file_system.h"

#include <Arduino.h>
#include <either.h>
#include <utility.h>
#include <memory.h>

#include "char_string.h"
#include "file.h"
#include "identifiers.h"
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

void FileSystem::format(const CharString& encryption_iv, const CharString& challenge)
{
    auto total_inodes = _eeprom->size / INODE_SIZE;
    for (auto index = 1u; index < total_inodes; index++)
        free_inode(inode_to_address(index));

    _master_block = FSMasterBlock(total_inodes - 1u, 0u, encryption_iv, challenge);
    write_master_block();
    delay(50);
    sync_usage_record();
}

void FileSystem::write_master_block()
{
    _ostream.seekg(0);
    _ostream << FSMasterINode(0u, false, _master_block);
}

void FileSystem::sync_usage_record()
{
    _istream.seekg(0);
    auto inode = FSMasterINode();
    _istream >> inode;
    _master_block = std::move(inode.data);
}

either<FileId, FileSystemError> FileSystem::write(const File& file)
{
    auto file_size = size(file);

    if (file_size > count_free_space())
        return FileSystemError::NotEnoughDiskSpace;

    auto to_write = write_file_to_string(file);
    auto inode_address = request_free_inode();
    auto fileId = FileId(address_to_inode(inode_address));

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
    return fileId;
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
        is_file_header,
        std::move(data_to_write));

    _ostream << inode_to_write;

    if (is_file_header)
        _master_block.file_headers++;
}

either<File, FileSystemError> FileSystem::read(const FileId& fileId)
{
    auto address = inode_to_address(fileId.value);
    auto header = read_inode_header(address);
    if (!header.flags.is_file_header)
        return FileSystemError::FileNotFound;
    return read_address_to_file(address);
}

either<File, FileSystemError> FileSystem::read(const CharString& filename)
{
    return get_fileid_by_filename(filename)
        .foldFirst([&] (auto&& file_id) { return read(file_id); });
}

either<FileId, FileSystemError> FileSystem::get_fileid_by_filename(const CharString& filename)
{
    auto file_count = static_cast<unsigned int>(count_files());
    auto file_address = 0u;
    auto file_id = FileId(0u);

    for (auto i = 0u; i < file_count && file_id.value == 0u; i++)
    {
        get_next_file_header(file_address)
            .match(
                [&](const auto& current_file_id) -> void
                {
                    get_filename(current_file_id)
                        .match(
                            [&] (const auto& current_filename) -> void
                            {
                                if (current_filename == filename) { file_id = current_file_id; }
                            },
                            [] (const auto&) { });
                    file_address = inode_to_address(current_file_id.value) + INODE_SIZE;
                },
                [] (const auto&) { });
    }

    if (file_id.value == 0u)
        return FileSystemError::FileNotFound;
    return file_id;
}


either<CharString, FileSystemError> FileSystem::get_filename(const FileId& fileId)
{
    return read(fileId)
        .mapFirst([] (const auto& file) { return file.name; });
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

either<unsigned int, FileSystemError> FileSystem::remove(const FileId& fileId)
{
    auto header = read_inode_header(inode_to_address(fileId.value));
    if (!header.flags.is_file_header)
        return FileSystemError::FileNotFound;

    auto address = inode_to_address(fileId.value);
    auto next_address = header.next;
    free_inode(address);

    while (next_address != 0)
    {
        header = read_inode_header(next_address);
        free_inode(next_address);
        next_address = header.next;
    }

    write_master_block();

    return address;
}

vector<FileId> FileSystem::list_files()
{
    auto file_count = static_cast<unsigned int>(count_files());
    auto filenames = vector<FileId>(file_count);
    auto file_address = 0u;
    for (auto i = 0u; i < file_count; i++)
    {
        get_next_file_header(file_address)
            .match(
                [&](const auto& fileId) -> void
                {
                    filenames.push_back(fileId);
                    file_address = inode_to_address(fileId.value) + INODE_SIZE;
                },
                [] (const auto&) { });
    }

    return filenames;
}

unsigned int FileSystem::request_free_inode()
{
    auto inode_count = _eeprom->size / INODE_SIZE;
    for (auto index = 1u; index < inode_count; index++)
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

either<FileId, FileSystemError> FileSystem::get_next_file_header(unsigned int starting_address)
{
    if (_master_block.file_headers == 0u)
        return FileSystemError::FileNotFound;

    auto inode_count = _eeprom->size / INODE_SIZE;
    for (auto index = address_to_inode(starting_address); index < inode_count; index++)
    {
        auto address = inode_to_address(index);
        auto header = read_inode_header(address);
        if (header.flags.is_file_header)
            return FileId(index);
    }

    return FileSystemError::FileNotFound;
}
