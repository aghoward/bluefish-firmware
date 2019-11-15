#pragma once

#include <Arduino.h>

enum class Command : byte
{
    Unknown = 0u,
    WriteFile,
    ReadFile,
    GetMasterBlock,
    ListFiles,
    RemoveFile,
    Format,
    GetFileName
};
        
class API
{
    protected:
        virtual void unknown_command() = 0;
        virtual void write_file() = 0;
        virtual void read_file() = 0;
        virtual void get_filename() = 0;
        virtual void get_master_block() = 0;
        virtual void list_files() = 0;
        virtual void remove_file() = 0;
        virtual void format() = 0;

    public:
        virtual ~API() {}
        
        void process_command(Command cmd);
        virtual Command read_command() = 0;
        virtual void notify_ready() = 0;
};

