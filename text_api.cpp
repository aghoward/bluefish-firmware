#include "text_api.h"
#include "char_string.h"
#include "file.h"

#include <Arduino.h>

Command TextAPI::read_command()
{
    auto command = Serial.readStringUntil('\n');
    command.trim();
    if (command.equals("format"))
        return Command::Format;
    else if (command.equals("write_file"))
        return Command::WriteFile;
    else if (command.equals("read_file"))
        return Command::ReadFile;
    else if (command.equals("print_usage"))
        return Command::PrintUsage;
    else if (command.equals("list"))
        return Command::ListFiles;
    else if (command.equals("remove_file"))
        return Command::RemoveFile;
    else
        return Command::Unknown;
}

CharString TextAPI::prompt_user(const char* prompt) const
{
    Serial.print(prompt);
    while (!Serial.available()) {}
    auto result = Serial.readStringUntil('\n');
    result.trim();
    return CharString(result.c_str());
}

void TextAPI::unknown_command()
{
    Serial.println("Error: Unrecognized command");
}

void TextAPI::write_file()
{
    auto file = File(
        prompt_user("Filename: "),
        "foo bar",
        "baz quz qux some more random non sensical words to fill up space so this bleeds over");
    auto result = _fs->write(file);

    result.match(
        [] (auto&&) {
            Serial.println("wrote file successfully");
            Serial.flush();
        },
        [] (auto&& error) {
            Serial.print("Write failed with error: ");
            Serial.flush();
            Serial.println(static_cast<unsigned>(error));
            Serial.flush();
        }
    );
}

void TextAPI::read_file()
{
    auto result = _fs->read(prompt_user("Filename: "));
    result.match(
        [] (auto&& file) {
            Serial.print("Filename @0x");
            Serial.print((unsigned int)(file.name.data()), HEX);
            Serial.print(": ");
            Serial.write(file.name.data(), file.name.size() - sizeof(unsigned int));
            Serial.println();
            Serial.flush();

            Serial.print("Username @0x");
            Serial.print((unsigned int)(file.username.data()), HEX);
            Serial.print(": ");
            Serial.write(file.username.data(), file.username.size() - sizeof(unsigned int));
            Serial.println();
            Serial.flush();

            Serial.print("Password @0x");
            Serial.print((unsigned int)(file.password.data()), HEX);
            Serial.print(": ");
            Serial.write(file.password.data(), file.password.size() - sizeof(unsigned int));
            Serial.println();
            Serial.flush();

        },
        [] (auto&& error) {
            Serial.print("Error reading file: 0x");
            Serial.print(static_cast<unsigned int>(error), HEX);
        });
}

void TextAPI::list_files()
{
    auto files = _fs->list_files();
    for (const auto& file : files)
    {
        Serial.print("Found file: ");
        Serial.write(file.data(), file.size() - 2);
        Serial.println();
        Serial.flush();
    }
}

void TextAPI::print_usage()
{
    const auto& master_block = _fs->get_usage();

    Serial.print("usage: ");
    Serial.println(master_block.usage_record);
    Serial.print("file usage: ");
    Serial.println(master_block.file_headers);
    Serial.flush();
}

void TextAPI::remove_file()
{
    _fs->remove(prompt_user("Filename: "))
        .match(
            [] (auto&&)
            {
                Serial.println("File removed successfully!");
                Serial.flush();
            },
            [] (auto&& error)
            {
                Serial.print("Error: ");
                Serial.println(static_cast<unsigned>(error));
                Serial.flush();
            });
}

void TextAPI::format()
{
    _fs->format();
}
