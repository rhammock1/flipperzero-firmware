/*
  Chip datasheet: https://ww1.microchip.com/downloads/en/DeviceDoc/25095A.pdf
*/

#include <mcp9808.h>
#include <mcp9808_reg.h>

#include <furi_hal_delay.h>
#include <furi/log.h>
#include <stdbool.h>

typedef struct {
  ConfigReg config_reg;
  TemperatureReg temp_reg;
} MCP9808_Registers;

static MCP9808_Registers mcp9808_regs;

void mcp9808_init(FuriHalI2cBusHandle* handle) {
  // Power on sensor
  if(!furi_hal_i2c_write_reg_16(
    handle,
    MCP9808_WRITE_ADDRESS,
    MCP9808_CONFIG_REG,
    *(uint16_t*)&mcp9808_regs.config_reg,
    MCP9808_I2C_TIMEOUT
  )) {
    FURI_LOG_E("MCP9808", "Failed to power on sensor");
  }
}

void mcp9808_poweroff(FuriHalI2cBusHandle* handle) {
  mcp9808_regs.config_reg.ShutdownMode = 1;
  if(!furi_hal_i2c_write_reg_16(
    handle,
    MCP9808_WRITE_ADDRESS,
    MCP9808_CONFIG_REG,
    *(uint16_t*)&mcp9808_regs.config_reg,
    MCP9808_I2C_TIMEOUT
  )) {
    FURI_LOG_E("MCP9808", "Failed to power off sensor");
  }
}

float mcp9808_read_temp(FuriHalI2cBusHandle* handle) {
  if(furi_hal_i2c_read_reg_16(
    handle,
    MCP9808_READ_ADDRESS,
    MCP9808_AMBIENT_TEMP_REG,
    (uint16_t*)&mcp9808_regs.temp_reg,
    MCP9808_I2C_TIMEOUT
  )) {
  float temp = 0.0;
  if(mcp9808_regs.temp_reg.BothBytes != 0xFFFF) {
    temp = mcp9808_regs.temp_reg.BothBytes & 0x0FFF;
    temp /= 16.0;
    if(mcp9808_regs.temp_reg.BothBytes & 0x1000) {
      temp -= 256.0;
    }
    // convert to Fahrenheit
    temp = (temp * 9.0 / 5.0) + 32.0;
  }
  return temp;
  // FURI_LOG_I("MCP9808", "UpperByte: %x LowerByte: %x", mcp9808_regs.temp_reg.UpperByte, mcp9808_regs.temp_reg.LowerByte);

  // mcp9808_regs.temp_reg.UpperByte &= 0x1F;
  // if((mcp9808_regs.temp_reg.UpperByte & 0x10) == 0x10) {
  //   mcp9808_regs.temp_reg.UpperByte &= 0x0F;
  //   FURI_LOG_I("MCP9808", "UpperByte: %f LowerByte: %f", (mcp9808_regs.temp_reg.UpperByte * 16), (mcp9808_regs.temp_reg.LowerByte / 16));
  //   temp = 256 - (mcp9808_regs.temp_reg.UpperByte * 16 + mcp9808_regs.temp_reg.LowerByte / 16);
  //   return temp;
  // } else {
  //   FURI_LOG_I("MCP9808", "UpperByte: %f LowerByte: %f", (mcp9808_regs.temp_reg.UpperByte * 16), (mcp9808_regs.temp_reg.LowerByte / 16));
  //   temp = (mcp9808_regs.temp_reg.UpperByte * 16) + (mcp9808_regs.temp_reg.LowerByte / 16);
  //   return temp;
  //  }
  } else {
    FURI_LOG_E("MCP9808", "Read error");
    return 0.0;
  }
}
