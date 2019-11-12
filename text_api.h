#pragma once

#include "api.h"
#include "char_string.h"
#include "eeprom.h"
#include "file_system.h"

#include <memory.h>

class TextAPI : public API
{
    private:
        std::unique_ptr<FileSystem> _fs;

        CharString prompt_user(const char* prompt) const;
    public:
        TextAPI()
            : _fs(std::make_unique<FileSystem>(std::make_unique<EEPROM>()))
        {} 

        Command read_command() override;

    protected:
        void unknown_command() override;
        void write_file() override;
        void read_file() override;
        void get_master_block() override;
        void list_files() override;
        void remove_file() override;
        void format() override;
};
