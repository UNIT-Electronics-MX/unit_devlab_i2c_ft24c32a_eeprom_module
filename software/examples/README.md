# Examples for FT24C32A EEPROM Module

This directory contains example code demonstrating how to use the FT24C32A EEPROM module with various microcontrollers.

## Available Examples

### [eeprom_test](eeprom_test/)
A comprehensive test and utility program for the FT24C32A EEPROM module with I2C interface.

Features:
- I2C device scanning and auto-detection
- Sequential read/write operations
- Paged write support (32-byte pages)
- Complete EEPROM erase functionality
- Write protection checking
- Serial command interface
- Hexadecimal and ASCII data display

## Hardware Setup

### Pin Configuration
Configure your I2C pins in the example code:

cpp
#define SDA_PIN 6  // I2C Data pin
#define SCL_PIN 7  // I2C Clock pin


### Supported Platforms
- ESP32C6/ESP8266: Uses custom SDA/SCL pins
- Other Arduino boards: Uses board-specific I2C pins

## Quick Start

1. Hardware Connection:
    - Connect VCC to 3.3V or 5V
    - Connect GND to ground
    - Connect SDA to your defined SDA pin
    - Connect SCL to your defined SCL pin

2. Upload the Code:
    - Open [eeprom_test.ino](eeprom_test/eeprom_test.ino) in Arduino IDE
    - Configure your I2C pins if needed
    - Upload to your microcontroller

3. Open Serial Monitor:
    - Set baud rate to 115200
    - Type scan to detect the EEPROM

## Usage Examples

### Basic Commands

| Command | Description | Example |
|---------|-------------|---------|
| scan | Scan for I2C devices and detect EEPROM | scan |
| read <addr> <length> | Read data from memory address | read 0x0000 16 |
| write <addr> <text> | Write text to memory address | write 0x0000 Hello World |
| erase | Erase entire EEPROM (fill with 0xFF) | erase |
| wpcheck [addr] | Check write protection status | wpcheck 0x0000 |
| addr <address> | Set EEPROM I2C address manually | addr 0x50 |

### Example Session


Monitor I2C-EEPROM listo. Escribe 'scan' para detectar la EEPROM.
scan

[I2C] Escaneando...
  - Dispositivo en 0x50  (EEPROM candidata)
Usando EEPROM en 0x50

write 0x0000 Hello UNIT Electronics!
Escrito en 0x0000

read 0x0000 22
HEX:
48 65 6C 6C 6F 20 55 4E 49 54 20 45 6C 65 63 74 
72 6F 6E 69 63 73 21 

ASCII: Hello UNIT Electronics!


## EEPROM Specifications

| Parameter | Value |
|-----------|-------|
| Capacity | 32 Kbits (4,096 bytes) |
| Interface | I2C (2-wire) |
| I2C Address | 0x50 - 0x57 (A0, A1, A2 configurable) |
| Page Size | 32 bytes |
| Operating Voltage | 1.8V - 5.5V |
| Max Clock Speed | 400 kHz |
| Write Cycle Time | ≤ 5ms |
| Data Retention | > 40 years |
| Endurance | > 1,000,000 cycles |

## Key Functions

### Core EEPROM Functions
- [eepromWaitReady()](eeprom_test/eeprom_test.ino): Wait for EEPROM to complete write cycle
- [eepromWritePaged()](eeprom_test/eeprom_test.ino): Write data with automatic page boundary handling
- [eepromReadSeq()](eeprom_test/eeprom_test.ino): Read sequential data from EEPROM

### Utility Functions
- [cmdScan()](eeprom_test/eeprom_test.ino): Scan I2C bus for devices
- [cmdRead()](eeprom_test/eeprom_test.ino): Read and display data
- [cmdWrite()](eeprom_test/eeprom_test.ino): Write text to EEPROM
- [cmdErase()](eeprom_test/eeprom_test.ino): Erase entire EEPROM
- [dumpHex()](eeprom_test/eeprom_test.ino): Display data in hexadecimal format

## Important Notes

1. Write Protection: Check the WP pin status if write operations fail
2. Page Boundaries: The EEPROM has 32-byte pages. Cross-page writes are automatically handled
3. Write Timing: Allow time between writes (handled automatically by eepromWaitReady())
4. Address Range: Valid addresses are 0x0000 to 0x0FFF (4095 decimal)
5. I2C Pull-ups: Ensure proper pull-up resistors on SDA/SCL lines (typically 4.7kΩ)

## Troubleshooting

| Issue | Solution |
|-------|----------|
| EEPROM not detected | Check wiring, I2C address, and pull-up resistors |
| Write errors | Verify WP pin is not high, check power supply |
| Communication errors | Reduce I2C clock speed, check connections |
| Data corruption | Ensure proper timing between operations |

## Additional Resources

- [Main Project README](../../README.md)
- [Hardware Documentation](../../hardware/)

## Contributing

Feel free to submit improvements or additional examples for different use cases!