/*
 * Copyright (c) 2026 cysoft
 * All rights reserved.
 */

#ifndef MS5837_H
#define MS5837_H

#include <stdint.h>
#include "esp_err.h"
#include "driver/i2c_master.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    i2c_master_dev_handle_t i2c_dev;
    uint16_t C[8]; // Calibration coefficients
} ms5837_handle_t;

typedef struct {
    float pressure_mbar;    // Pressure in mbar
    float temperature_c;    // Temperature in Celsius
} ms5837_data_t;

/**
 * @brief Initialize the MS5837 sensor
 * 
 * @param[in] bus_handle Handle to the I2C master bus
 * @param[in] address I2C address of the sensor (usually 0x76)
 * @param[out] out_handle Pointer to allocate driver handle
 * @return esp_err_t ESP_OK on success
 */
esp_err_t ms5837_init(i2c_master_bus_handle_t bus_handle, uint8_t address, ms5837_handle_t **out_handle);

/**
 * @brief Read pressure and temperature from the sensor
 * 
 * @param[in] handle Driver handle
 * @param[out] data Pointer to store results
 * @return esp_err_t ESP_OK on success
 */
esp_err_t ms5837_read(ms5837_handle_t *handle, ms5837_data_t *out_data);

#ifdef __cplusplus
}
#endif

#endif // MS5837_H
