#include "binary_api.h"

#include <utility.h>

#include "identifiers.h"
#include "stream.h"

CommandStatus BinaryAPI::convert_error(FileSystemError error) const
{
    if (error == FileSystemError::NotEnoughDiskSpace)
        return CommandStatus::NotEnoughDiskSpace;
    else if (error == FileSystemError::FileNotFound)
        return CommandStatus::FileNotFound;
    return CommandStatus::Fail;
}

void BinaryAPI::notify_ready()
{
    _output.put(static_cast<byte>(CommandStatus::Ready));
}

Command BinaryAPI::read_command()
{
    notify_ready();
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
    FileId id;
    _input >> id;
    _fs->read(id)
        .match(
            [&](auto&& file) {
                _output.put(static_cast<byte>(CommandStatus::OK));
                _output << file;
            },
            [&](auto&& error) { _output.put(static_cast<byte>(convert_error(error))); }
        );
}

void BinaryAPI::get_filename()
{
    FileId id;
    _input >> id;
    _fs->get_filename(id)
        .match(
            [&] (auto&& filename) {
                _output.put(static_cast<byte>(CommandStatus::OK));
                _output << filename;
            },
            [&](auto&& error) { _output.put(static_cast<byte>(convert_error(error))); }
        );
}

void BinaryAPI::get_master_block()
{
    const auto& master_block = _fs->get_master_block();
    _output << master_block;
}

void BinaryAPI::list_files()
{
    auto fileIds = _fs->list_files();
    _output.put(static_cast<byte>(fileIds.size()));
    for (auto& id : fileIds)
        _output << id;
}

void BinaryAPI::remove_file()
{
    FileId fileId;
    _input >> fileId;
    _fs->remove(fileId)
        .match(
            [&](auto&&) { _output.put(static_cast<byte>(CommandStatus::OK)); },
            [&](auto&& error) { _output.put(static_cast<byte>(convert_error(error))); }
        );
}

void BinaryAPI::format()
{
    CharString encryption_iv, challenge;
    _input >> encryption_iv >> challenge;
    _fs->format(std::move(encryption_iv), std::move(challenge));
    _output.put(static_cast<byte>(CommandStatus::OK));
}
