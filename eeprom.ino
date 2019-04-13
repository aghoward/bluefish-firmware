#include <Arduino.h>
#include <Wire.h>
#include <memory.h>

#include "24lc16b.h"
#include "file_system.h"
#include "file.h"
#include "char_string.h"

std::unique_ptr<FileSystem> fs;

void write_file()
{
    Serial.print("Filename: ");
    while (!Serial.available()) {}
    
    auto filename = Serial.readStringUntil('\n');
    filename.trim();

    auto file = File(
        filename.c_str(),
        "foo bar",
        "baz quz qux some more random non sensical words to fill up space so this bleeds over");
    auto result = fs->write(file);

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

void read_file()
{
    Serial.print("Filename: ");
    while (!Serial.available()) {}
    auto filename = Serial.readStringUntil('\n');
    filename.trim();

    auto result = fs->read(filename.c_str());
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

void list_files()
{
    auto files = fs->list_files();
    for (const auto& file : files)
    {
        Serial.print("Found file: ");
        Serial.write(file.data(), file.size() - 2);
        Serial.println();
        Serial.flush();
    }
}

void setup()
{
    Wire.begin();

    Serial.begin(115200);
    while (!Serial) {}
    Serial.println("Begin...");

    fs = std::make_unique<FileSystem>(std::make_unique<EEPROM>());
    Serial.println("Filesystem initialized!");
}

void loop()
{
    while (!Serial.available()) {}

    auto command = Serial.readStringUntil('\n');
    command.trim();
    if (command.equals("format"))
    {
        fs->format();
        Serial.println("Formatted!");
    }
    else if (command.equals("write_file"))
        write_file();
    else if (command.equals("read_file"))
        read_file();
    else if (command.equals("print_usage"))
        fs->print_usage();
    else if (command.equals("list"))
        list_files();
    else
    {
        Serial.print("Error: Unrecognized command: ");
        Serial.println(command);
    }
}
