#pragma once

typedef struct {
  // Bits 11 - 15 are unimplemented
  uint8_t Res: 5; // Reserved bits
  // If locked, the threshold values are not updated
  uint8_t AlertLower_t: 1; // Alert Lower Threshold bit
  uint8_t AlertUpper_t: 1; // Alert Upper Threshold bit
  // Low Power Mode
  uint8_t ShutdownMode: 1; // Shutdown Mode bit
  // Threshold locks
  uint8_t CritLock: 1; // Critical Lock bit
  uint8_t WinLock: 1; // Window Lock bit
  // Threshold Alert Configs
  uint8_t IntClear: 1; // Interrupt Clear bit
  uint8_t AlertStat: 1; // Alert Output Status bit
  uint8_t AlertCnt: 1; // Alert Output Control bit
  uint8_t AlertSel: 1; // Alert Output Select bit
  uint8_t AlertPol: 1; // Alert Output Polarity bit
  uint8_t AlertMod: 1; // Alert Output Mode bit
} ConfigReg;

typedef struct {
  // Read-only Register
  uint16_t BothBytes: 16; // Both bytes of the temperature

  // The way the datasheet says to do it
  // uint8_t LowerByte: 8; // Lower byte of temperature
  // uint8_t UpperByte: 8; // Upper byte of temperature
} TemperatureReg;

#define MCP9808_READ_ADDRESS 0b00110001
#define MCP9808_WRITE_ADDRESS 0b00110000
#define MCP9808_I2C_TIMEOUT 200

#define MCP9808_CONFIG_REG 0x01
#define MCP9808_AMBIENT_TEMP_REG 0x05