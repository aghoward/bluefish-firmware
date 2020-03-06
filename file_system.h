#pragma once

#include <memory.h>
#include <utility.h>

#include <either.h>

#include "char_string.h"
#include "eeprom.h"
#include "file.h"
#include "fs_master_block.h"
#include "identifiers.h"
#include "stream.h"
#include "vector.h"

enum class FileSystemError
{
    NotEnoughDiskSpace,
    FileNotFound
};

class FileSystem
{
    private:
        std::unique_ptr<EEPROM> _eeprom;
        istream _istream;
        ostream _ostream;
        size_t _inode_count;

        FSMasterBlock _master_block;

        void sync_usage_record();

        unsigned int request_free_inode();
        void free_inode(unsigned int address);

        either<FileId, FileSystemError> get_next_file_header(unsigned int starting_address);
        either<FileId, FileSystemError> get_fileid_by_filename(const CharString& filename);

        File read_address_to_file(unsigned int address);
        CharString read_file_to_string(unsigned int address);
        INode<CharString> read_file_inode(unsigned int address);
        INode<void> read_inode_header(unsigned int address);

        CharString write_file_to_string(const File& file) const;
        void write_inode(
                unsigned int inode_address,
                unsigned int next_address,
                unsigned int start_index,
                unsigned int bytes_to_write,
                const CharString& to_write);

    public:
        FileSystem(std::unique_ptr<EEPROM>&& eeprom)
            : _eeprom(std::move(eeprom)),
            _istream(_eeprom.get()),
            _ostream(_eeprom.get()),
            _inode_count(_eeprom->size / INODE_SIZE),
            _master_block()
        {
            sync_usage_record();
        }

        void write_master_block();
        size_t count_free_space();
        size_t count_files();

        void format(const CharString& encryption_iv, const CharString& challenge);
        const FSMasterBlock& get_master_block() const;
        either<FileId, FileSystemError> write(const File& file);
        either<File, FileSystemError> read(const CharString& filename);
        either<File, FileSystemError> read(const FileId& fileId);
        either<CharString, FileSystemError> get_filename(const FileId& fileId);
        either<unsigned int, FileSystemError> remove(const FileId& fileId);
        vector<FileId> list_files();
};
