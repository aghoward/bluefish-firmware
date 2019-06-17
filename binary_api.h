#pragma once

#include <Arduino.h>

#include "api.h"
#include "file_system.h"
#include "serial_stream.h"
#include "stream.h"

enum class CommandStatus : byte
{
    OK = 0u,
    Fail,
    NotEnoughDiskSpace,
    FileNotFound
};

class BinaryAPI : public API
{
    private:
        std::unique_ptr<FileSystem> _fs;
        SerialStream _stream;
        istream _input;
        ostream _output;

    protected:
        void unknown_command();
        void write_file();
        void read_file();
        void print_usage();
        void list_files();
        void remove_file();
        void format();

        CommandStatus convert_error(FileSystemError error) const;

    public:
        BinaryAPI()
            : _fs(std::make_unique<FileSystem>(std::make_unique<EEPROM>())),
            _stream(),
            _input(&_stream),
            _output(&_stream)
        {}

        Command read_command();
};
