#include <SPI.h>
#include <XRA1405.hpp>

// Define SPI pins
#define SCK 18
#define MISO 19
#define MOSI 23
#define SS 5

void setup()
{
    Serial.begin(115200);

    // Initialize SPI and XRA1405
    XRA1405_begin(SCK, MISO, MOSI);
    XRA1405_pinMode(SS, 3, OUTPUT);
    XRA1405_digitalWrite(SS, 3, HIGH);

    Serial.println("XRA1405 initialized and pin 3 set HIGH");
}

void loop()
{
    // Add logic to read and write GPIO states as needed
}
