#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <furi_hal_i2c.h>
#include <mcp9808_reg.h>

/** Initialize Driver */
void mcp9808_init(FuriHalI2cBusHandle* handle);

/** Shutdown Sensor */
void mcp9808_poweroff(FuriHalI2cBusHandle* handle);

/** Read Temperature */
float mcp9808_read_temp(FuriHalI2cBusHandle* handle);
