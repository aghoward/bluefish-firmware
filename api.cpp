#include "api.h"


void API::process_command(Command cmd)
{
    if (cmd == Command::Unknown)
        this->unknown_command();
    else if (cmd == Command::WriteFile)
        this->write_file();
    else if (cmd == Command::ReadFile)
        this->read_file();
    else if (cmd == Command::PrintUsage)
        this->print_usage();
    else if (cmd == Command::ListFiles)
        this->list_files();
    else if (cmd == Command::RemoveFile)
        this->remove_file();
    else if (cmd == Command::Format)
        this->format();
}
