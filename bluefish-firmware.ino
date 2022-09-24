#include <Arduino.h>
#include <Wire.h>
#include <memory.h>

#include "file_system.h"
#include "file.h"
#include "char_string.h"

#include "api.h"
#include "binary_api.h"

std::unique_ptr<API> _api;

void setup()
{
    pinMode(2, OUTPUT);
    pinMode(3, OUTPUT);
    Wire.begin();
    Wire.setClock(400000);

    digitalWrite(2, HIGH);
    Serial.begin(115200);
    while (!Serial) {}

    _api = std::make_unique<BinaryAPI>();
    digitalWrite(2, LOW);
}

void loop()
{
    auto command = _api->read_command();
    _api->process_command(command);
}
