#pragma once

#include <Arduino.h>

enum class Command : byte
{
    Unknown = 0u,
    WriteFile,
    ReadFile,
    PrintUsage,
    ListFiles,
    RemoveFile,
    Format
};
        
class API
{
    protected:
        virtual void unknown_command() = 0;
        virtual void write_file() = 0;
        virtual void read_file() = 0;
        virtual void print_usage() = 0;
        virtual void list_files() = 0;
        virtual void remove_file() = 0;
        virtual void format() = 0;

    public:
        virtual ~API() {}
        
        void process_command(Command cmd);
        virtual Command read_command() = 0;
};

