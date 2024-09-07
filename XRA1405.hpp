/**
 * @file
 *    XRA1405 Library
 *
 * @brief
 *    This library provides an interface for controlling the XRA1405 SPI GPIO expander,
 *    enabling management of GPIO pins for input, output, and other configurations over SPI. It is ideal for
 *    expanding the number of GPIOs available in a microcontroller-based project.
 *
 *    The library supports:
 *    - Initializing the SPI bus
 *    - Setting pin modes
 *    - Reading and writing digital values to pins
 *    - Enabling or disabling internal pull-up resistors
 *    - Configuring interrupts for GPIO pins
 *    - Clearing triggered interrupts
 *
 *    SPI Command Byte Format:
 *    - Bit 7 for Read/Write (1 for Read, 0 for Write)
 *    - Bits 6:1 for the Command Byte (the register address)
 *    - Bit 0 is reserved
 *
 * Usage and Examples:
 *    Initialize the SPI bus:
 *      `XRA1405_begin(26000000, SCK, MISO, MOSI, SS); // Initialize SPI with 26MHz clock`
 *
 *    Set the mode of a GPIO pin:
 *      `XRA1405_pinMode(SS, 3, OUTPUT); // Set pin 3 as an output`
 *
 *    Write to a GPIO pin:
 *      `XRA1405_digitalWrite(SS, 3, HIGH); // Set pin 3 high`
 *
 *    Read from a GPIO pin:
 *      `uint8_t pinState = XRA1405_digitalRead(SS, 3); // Read the state of pin 3`
 *
 *    Enable the internal pull-up resistor for a GPIO pin:
 *      `XRA1405_setPullUp(SS, 3, true); // Enable pull-up resistor for pin 3`
 *
 *    Configure the interrupt for a GPIO pin:
 *      `XRA1405_setInterrupt(SS, 3, INTERRUPT_RISING); // Set rising edge interrupt for pin 3`
 *
 *    Clear any triggered interrupts:
 *      `XRA1405_clearInterrupts(SS); // Clear all triggered interrupts`
 *
 * @author
 *    Itay Nave, Embedded Software Engineer
 * @date
 *    03/12/2024
 *
 * @copyright
 *    © 2023 Itay Nave. All rights reserved.
 *    This software is provided "as is", without warranty of any kind, express or implied.
 *    Redistribution and use in source and binary forms, with or without modification,
 *    are permitted provided that the above copyright notice and this permission notice
 *    appear in all copies.
 */

#ifndef XRA1405_HPP
#define XRA1405_HPP

#include <Arduino.h>
#include <SPI.h>

#define XRA1405_SPI_CLOCK 26000000 // 26 MHz
#define SPI_ORDER MSBFIRST
#define SPI_MODE SPI_MODE0

#define XRA1405_WRITE B01111111
#define XRA1405_READ B10000000

// Enum for XRA1405 register addresses with pre-shifted values for direct use in SPI command byte
enum XRA1405_Register
{
    GSR1 = 0x00 << 1,  // GPIO State Register for P0-P7
    GSR2 = 0x01 << 1,  // GPIO State Register for P8-P15
    OCR1 = 0x02 << 1,  // Output Control Register for P0-P7
    OCR2 = 0x03 << 1,  // Output Control Register for P8-P15
    PIR1 = 0x04 << 1,  // Input Polarity Inversion Register for P0-P7
    PIR2 = 0x05 << 1,  // Input Polarity Inversion Register for P8-P15
    GCR1 = 0x06 << 1,  // GPIO Configuration Register for P0-P7
    GCR2 = 0x07 << 1,  // GPIO Configuration Register for P8-P15
    PUR1 = 0x08 << 1,  // Pull-up Resistor Enable Register for P0-P7
    PUR2 = 0x09 << 1,  // Pull-up Resistor Enable Register for P8-P15
    IER1 = 0x0A << 1,  // Input Interrupt Enable Register for P0-P7
    IER2 = 0x0B << 1,  // Input Interrupt Enable Register for P8-P15
    TSCR1 = 0x0C << 1, // Three-State Control Register for P0-P7
    TSCR2 = 0x0D << 1, // Three-State Control Register for P8-P15
    ISR1 = 0x0E << 1,  // Input Interrupt Status Register for P0-P7
    ISR2 = 0x0F << 1,  // Input Interrupt Status Register for P8-P15
    REIR1 = 0x10 << 1, // Rising Edge Interrupt Enable Register for P0-P7
    REIR2 = 0x11 << 1, // Rising Edge Interrupt Enable Register for P8-P15
    FEIR1 = 0x12 << 1, // Falling Edge Interrupt Enable Register for P0-P7
    FEIR2 = 0x13 << 1, // Falling Edge Interrupt Enable Register for P8-P15
    IFR1 = 0x14 << 1,  // Input Filter Enable Register for P0-P7
    IFR2 = 0x15 << 1   // Input Filter Enable Register for P8-P15
};

// Enum for interrupt types
enum XRA1405_InterruptType
{
    INTERRUPT_DISABLE = 0, // Disable interrupts
    INTERRUPT_RISING,      // Enable rising edge interrupt
    INTERRUPT_FALLING,     // Enable falling edge interrupt
    INTERRUPT_BOTH         // Enable both rising and falling edge interrupts
};

// Setting the read or write mode in the command byte
static uint8_t SPI_Read(uint8_t chipSelectPin, uint8_t commandByte);
static void SPI_Write(uint8_t chipSelectPin, uint8_t commandByte, uint8_t dataByte);
static uint8_t setReadMode(uint8_t commandByte);
static uint8_t setWriteMode(uint8_t commandByte);
static void configureEdgeInterrupt(uint8_t chipSelectPin, uint8_t pin, XRA1405_InterruptType interruptType);

// Initialize the SPI bus (only needs to be done once)
void XRA1405_begin(int8_t sck, int8_t miso, int8_t mosi, uint32_t freq = 26000000);

// Set the mode of a GPIO pin (input, output, three-state)
void XRA1405_pinMode(uint8_t chipSelectPin, uint8_t pin, uint8_t mode);

// Write to a GPIO pin
void XRA1405_digitalWrite(uint8_t chipSelectPin, uint8_t pin, uint8_t value);

// Read from a GPIO pin
uint8_t XRA1405_digitalRead(uint8_t chipSelectPin, uint8_t pin);

// Enable/disable the internal pull-up resistor for a GPIO pin
void XRA1405_setPullUp(uint8_t chipSelectPin, uint8_t pin, bool enabled);

// Configure the interrupt for a GPIO pin
void XRA1405_setInterrupt(uint8_t chipSelectPin, uint8_t pin, XRA1405_InterruptType type);

// Clear any triggered interrupts
void XRA1405_clearInterrupts(uint8_t chipSelectPin);

#endif // XRA1405_HPP