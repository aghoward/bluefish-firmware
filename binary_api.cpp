#include "binary_api.h"

#include "stream.h"

CommandStatus BinaryAPI::convert_error(FileSystemError error) const
{
    if (error == FileSystemError::NotEnoughDiskSpace)
        return CommandStatus::NotEnoughDiskSpace;
    else if (error == FileSystemError::FileNotFound)
        return CommandStatus::FileNotFound;
    return CommandStatus::Fail;
}

Command BinaryAPI::read_command()
{
    _output.put(static_cast<byte>(CommandStatus::Ready));
    auto command = static_cast<byte>(_input.get());
    return static_cast<Command>(command);
}

void BinaryAPI::unknown_command()
{
    _output.put(static_cast<byte>(CommandStatus::Fail));
}

void BinaryAPI::write_file()
{
    File file;
    _input >> file;
    _fs->write(file)
        .match(
            [&](auto&&) { _output.put(static_cast<byte>(CommandStatus::OK)); },
            [&](auto&& error) { _output.put(static_cast<byte>(convert_error(error))); }
        );
}

void BinaryAPI::read_file()
{
    CharString filename;
    _input >> filename;
    _fs->read(filename)
        .match(
            [&](auto&& file) {
                _output.put(static_cast<byte>(CommandStatus::OK));
                _output << file;
            },
            [&](auto&& error) { _output.put(static_cast<byte>(convert_error(error))); }
        );
}

void BinaryAPI::print_usage()
{
    const auto& master_block = _fs->get_usage();
    _output << master_block;
}

void BinaryAPI::list_files()
{
    auto filenames = _fs->list_files();
    _output.put(static_cast<byte>(filenames.size()));
    for (auto& filename : filenames)
        _output << filename; 
}

void BinaryAPI::remove_file()
{
    CharString filename;
    _input >> filename;
    _fs->remove(filename)
        .match(
            [&](auto&&) { _output.put(static_cast<byte>(CommandStatus::OK)); },
            [&](auto&& error) { _output.put(static_cast<byte>(convert_error(error))); }
        );
}

void BinaryAPI::format()
{
    _fs->format();
    _output.put(static_cast<byte>(CommandStatus::OK));
}
