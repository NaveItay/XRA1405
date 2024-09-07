#include "XRA1405.hpp"

void XRA1405_begin(int8_t sck, int8_t miso, int8_t mosi, uint32_t freq)
{
    // Ensure the frequency is between 24 MHz and 26 MHz. If not, set to default (26 MHz)
    if (freq < 24000000 || freq > 26000000)
    {
        freq = XRA1405_SPI_CLOCK; // Default frequency
    }

    SPI.begin(sck, miso, mosi, -1); // Initialize the SPI bus with specified pins
    SPI.setFrequency(freq);         // Set the SPI clock frequency
}

void XRA1405_pinMode(uint8_t chipSelectPin, uint8_t pin, uint8_t mode)
{
    uint8_t gpioConfigRegisterCommand = (pin < 8) ? GCR1 : GCR2;
    pin %= 8; // Adjust pin number for 0-7 range

    // Use SPI_Read to get the current GPIO Configuration Register value
    uint8_t gpioConfigRegisterValue = SPI_Read(chipSelectPin, setReadMode(gpioConfigRegisterCommand));

    // Modify the GPIO Configuration Register value based on mode
    uint8_t newConfigValue = mode == OUTPUT ? (gpioConfigRegisterValue & ~(1 << pin)) : (gpioConfigRegisterValue | (1 << pin));

    // Use SPI_Write to write the modified value back
    SPI_Write(chipSelectPin, setWriteMode(gpioConfigRegisterCommand), newConfigValue);

    // If mode is INPUT_PULLUP, enable the pull-up resistor
    if (mode == INPUT_PULLUP)
    {
        uint8_t pullUpResistorRegisterCommand = (pin < 8) ? PUR1 : PUR2;
        uint8_t pullUpResistorRegisterValue = SPI_Read(chipSelectPin, setReadMode(pullUpResistorRegisterCommand));
        SPI_Write(chipSelectPin, setWriteMode(pullUpResistorRegisterCommand), pullUpResistorRegisterValue | (1 << pin));
    }
}

void XRA1405_digitalWrite(uint8_t chipSelectPin, uint8_t pin, uint8_t value)
{
    uint8_t outputControlRegisterCommand = (pin < 8) ? OCR1 : OCR2;
    pin %= 8; // Adjust pin number for 0-7 range after determining the register

    // Use SPI_Read to get the current state of the Output Control Register
    uint8_t outputControlRegisterValue = SPI_Read(chipSelectPin, setReadMode(outputControlRegisterCommand));

    // Modify the Output Control Register value based on the desired value
    uint8_t newOutputControlValue = value == HIGH ? (outputControlRegisterValue | (1 << pin)) : (outputControlRegisterValue & ~(1 << pin));

    // Use SPI_Write to write the modified value back to the Output Control Register
    SPI_Write(chipSelectPin, setWriteMode(outputControlRegisterCommand), newOutputControlValue);
}

uint8_t XRA1405_digitalRead(uint8_t chipSelectPin, uint8_t pin)
{
    uint8_t gpioStateRegisterCommand = (pin < 8) ? GSR1 : GSR2;
    pin %= 8; // Adjust pin number for 0-7 range after determining the register

    // Use SPI_Read to perform the SPI transaction and read the state from the GPIO State Register
    uint8_t gpioStateRegisterValue = SPI_Read(chipSelectPin, setReadMode(gpioStateRegisterCommand));

    // Extract and return the state of the specified pin from the GSR value
    return (gpioStateRegisterValue >> pin) & 0x01; // Shift the GSR value right and mask with 0x01 to isolate the pin state
}

void XRA1405_setPullUp(uint8_t chipSelectPin, uint8_t pin, bool enabled)
{
    // Determine which Pull-Up Resistor Register (PUR) to use based on pin number
    uint8_t pullUpResistorRegisterCommand = (pin < 8) ? PUR1 : PUR2;
    pin %= 8; // Adjust pin number for 0-7 range

    // Use SPI_Read to get the current state of the Pull-Up Resistor Register
    uint8_t pullUpResistorRegisterValue = SPI_Read(chipSelectPin, setReadMode(pullUpResistorRegisterCommand));

    // Modify the Pull-Up Resistor Register value based on whether the pull-up is enabled or not
    uint8_t newPullUpValue = enabled ? (pullUpResistorRegisterValue | (1 << pin)) : (pullUpResistorRegisterValue & ~(1 << pin));

    // Use SPI_Write to write the modified value back to the Pull-Up Resistor Register
    SPI_Write(chipSelectPin, setWriteMode(pullUpResistorRegisterCommand), newPullUpValue);
}

void XRA1405_setInterrupt(uint8_t chipSelectPin, uint8_t pin, XRA1405_InterruptType interruptType)
{
    uint8_t interruptEnableRegisterCommand = (pin < 8) ? IER1 : IER2;
    pin %= 8; // Adjust pin number for 0-7 range

    // Enable interrupt for the pin, regardless of edge detection, using SPI_Read and SPI_Write
    uint8_t interruptEnableRegisterValue = SPI_Read(chipSelectPin, setReadMode(interruptEnableRegisterCommand));
    interruptEnableRegisterValue |= (1 << pin);
    SPI_Write(chipSelectPin, setWriteMode(interruptEnableRegisterCommand), interruptEnableRegisterValue);

    // Configure edge detection based on the interruptType
    configureEdgeInterrupt(chipSelectPin, pin, interruptType);
}

void XRA1405_clearInterrupts(uint8_t chipSelectPin)
{
    uint8_t interruptStatusRegister1 = ISR1; // ISR1 for P0-P7
    uint8_t interruptStatusRegister2 = ISR2; // ISR2 for P8-P15

    // Clear interrupts for P0-P7 by reading the ISR1 register
    SPI_Read(chipSelectPin, setReadMode(interruptStatusRegister1));
    /// Note: Reading the interrupt status register typically clears the interrupt flags,
    /// @todo so there's no need to write back. However, confirm this behavior with the XRA1405 datasheet.

    // Clear interrupts for P8-P15 by reading the ISR2 register
    SPI_Read(chipSelectPin, setReadMode(interruptStatusRegister2));
    // Similarly, reading ISR2 should clear the interrupt flags for these pins.
}

static uint8_t SPI_Read(uint8_t chipSelectPin, uint8_t commandByte)
{
    digitalWrite(chipSelectPin, LOW);
    SPI.beginTransaction(SPISettings(XRA1405_SPI_CLOCK, SPI_ORDER, SPI_MODE));
    SPI.transfer(commandByte);              // Send the command byte with read mode set
    uint8_t readValue = SPI.transfer(0x00); // Clock out the read value
    SPI.endTransaction();
    digitalWrite(chipSelectPin, HIGH);

    return readValue;
}

static void SPI_Write(uint8_t chipSelectPin, uint8_t commandByte, uint8_t dataByte)
{
    digitalWrite(chipSelectPin, LOW);
    SPI.beginTransaction(SPISettings(XRA1405_SPI_CLOCK, SPI_ORDER, SPI_MODE));
    SPI.transfer(commandByte); // Send the command byte with write mode set
    SPI.transfer(dataByte);    // Send the data byte
    SPI.endTransaction();
    digitalWrite(chipSelectPin, HIGH);
}

static uint8_t setReadMode(uint8_t commandByte)
{
    // Set the MSB to 1 to indicate a read operation
    return commandByte | XRA1405_READ;
}

static uint8_t setWriteMode(uint8_t commandByte)
{
    // Ensure the MSB is 0 to indicate a write operation
    return commandByte & XRA1405_WRITE;
}

static void configureEdgeInterrupt(uint8_t chipSelectPin, uint8_t pin, XRA1405_InterruptType interruptType)
{
    uint8_t risingEdgeRegisterCommand = (pin < 8) ? REIR1 : REIR2;
    uint8_t fallingEdgeRegisterCommand = (pin < 8) ? FEIR1 : FEIR2;
    pin %= 8; // Adjust pin number for 0-7 range

    bool enableRising = (interruptType == INTERRUPT_RISING || interruptType == INTERRUPT_BOTH);
    bool enableFalling = (interruptType == INTERRUPT_FALLING || interruptType == INTERRUPT_BOTH);

    // Modify the Rising Edge Interrupt Register value
    uint8_t risingInterruptRegisterValue = SPI_Read(chipSelectPin, setReadMode(risingEdgeRegisterCommand));
    risingInterruptRegisterValue = enableRising ? (risingInterruptRegisterValue | (1 << pin)) : (risingInterruptRegisterValue & ~(1 << pin));
    SPI_Write(chipSelectPin, setWriteMode(risingEdgeRegisterCommand), risingInterruptRegisterValue);

    // Modify the Falling Edge Interrupt Register value
    uint8_t fallingInterruptRegisterValue = SPI_Read(chipSelectPin, setReadMode(fallingEdgeRegisterCommand));
    fallingInterruptRegisterValue = enableFalling ? (fallingInterruptRegisterValue | (1 << pin)) : (fallingInterruptRegisterValue & ~(1 << pin));
    SPI_Write(chipSelectPin, setWriteMode(fallingEdgeRegisterCommand), fallingInterruptRegisterValue);

    // If interruptType is DISABLE, disable both edge interrupts
    if (interruptType == INTERRUPT_DISABLE)
    {
        SPI_Write(chipSelectPin, setWriteMode(risingEdgeRegisterCommand), risingInterruptRegisterValue & ~(1 << pin));
        SPI_Write(chipSelectPin, setWriteMode(fallingEdgeRegisterCommand), fallingInterruptRegisterValue & ~(1 << pin));
    }
}