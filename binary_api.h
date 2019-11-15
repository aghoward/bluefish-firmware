#pragma once

#include <Arduino.h>

#include "api.h"
#include "file_system.h"
#include "serial_stream.h"
#include "stream.h"
#include "eeprom.h"

enum class CommandStatus : byte
{
    OK = 0u,
    Fail,
    NotEnoughDiskSpace,
    FileNotFound,
    Ready
};

class BinaryAPI : public API
{
    private:
        std::unique_ptr<FileSystem> _fs;
        SerialStream _sstream;
        istream _input;
        ostream _output;

    protected:
        void unknown_command() override;
        void write_file() override;
        void read_file() override;
        void get_filename() override;
        void get_master_block() override;
        void list_files() override;
        void remove_file() override;
        void format() override;

        CommandStatus convert_error(FileSystemError error) const;

    public:
        BinaryAPI()
            : _fs(std::make_unique<FileSystem>(std::make_unique<EEPROM>())),
            _sstream(),
            _input(&_sstream),
            _output(&_sstream)
        {
        }

        Command read_command() override;
        void notify_ready() override;
};
