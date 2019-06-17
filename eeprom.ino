#include <Arduino.h>
#include <Wire.h>
#include <memory.h>

#include "file_system.h"
#include "file.h"
#include "char_string.h"

#include "api.h"
#include "text_api.h"

std::unique_ptr<API> _api;

void setup()
{
    Wire.begin();

    Serial.begin(115200);
    while (!Serial) {}
    Serial.println("Begin...");

    _api = std::make_unique<TextAPI>();
    Serial.println("Filesystem initialized!");
}

void loop()
{
    while (!Serial.available()) {}

    auto command = _api->read_command();
    _api->process_command(command);
}
