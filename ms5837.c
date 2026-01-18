/*
 * Copyright (c) 2026 cuiy
 * All rights reserved.
 */

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_check.h"
#include "ms5837.h"

static const char *TAG = "MS5837";

#define MS5837_CMD_RESET 0x1E
#define MS5837_CMD_ADC_READ 0x00
#define MS5837_CMD_PROM_READ 0xA0
#define MS5837_CMD_CONVERT_D1_8192 0x4A // Max OSR
#define MS5837_CMD_CONVERT_D2_8192 0x5A // Max OSR

static esp_err_t ms5837_reset(ms5837_handle_t *handle) {
    uint8_t cmd = MS5837_CMD_RESET;
    return i2c_master_transmit(handle->i2c_dev, &cmd, 1, 1000);
}

static esp_err_t ms5837_read_prom(ms5837_handle_t *handle) {
    uint8_t cmd;
    uint8_t buffer[2];
    
    // Read C1 to C7 (C0 is CRC, usually not needed for calc but good to check. Datasheet says read 0-7)
    // We only strictly need C1-C6 for calculation.
    for (int i = 0; i < 7; i++) {
        cmd = MS5837_CMD_PROM_READ + (i * 2);
        ESP_RETURN_ON_ERROR(i2c_master_transmit_receive(handle->i2c_dev, &cmd, 1, buffer, 2, 1000), TAG, "Failed to read PROM");
        
        handle->C[i] = (buffer[0] << 8) | buffer[1];
        // ESP_LOGI(TAG, "C[%d] = %u", i, handle->C[i]);
    }
    return ESP_OK;
}

esp_err_t ms5837_init(i2c_master_bus_handle_t bus_handle, uint8_t address, ms5837_handle_t **out_handle) {
    ms5837_handle_t *handle = (ms5837_handle_t *)calloc(1, sizeof(ms5837_handle_t));
    if (!handle) return ESP_ERR_NO_MEM;

    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = address,
        .scl_speed_hz = 100000, // Standard mode, sensor supports up to 400k
    };

    esp_err_t ret = i2c_master_bus_add_device(bus_handle, &dev_cfg, &handle->i2c_dev);
    if (ret != ESP_OK) {
        free(handle);
        return ret;
    }

    // Reset sensor
    ESP_LOGI(TAG, "Resetting sensor...");
    int retry = 0;
    while (retry < 5) {
        ret = ms5837_reset(handle);
        if (ret == ESP_OK) {
            break;
        }
        ESP_LOGW(TAG, "Reset attempt %d failed (%s), retrying...", retry + 1, esp_err_to_name(ret));
        vTaskDelay(pdMS_TO_TICKS(100));
        retry++;
    }

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Reset failed: %s", esp_err_to_name(ret));
        i2c_master_bus_rm_device(handle->i2c_dev);
        free(handle);
        return ret;
    }
    
    // Wait for reset to complete
    vTaskDelay(pdMS_TO_TICKS(20));

    // Read calibration coefficients
    ESP_LOGI(TAG, "Reading calibration...");
    ret = ms5837_read_prom(handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Reading calibration failed: %s", esp_err_to_name(ret));
        i2c_master_bus_rm_device(handle->i2c_dev);
        free(handle);
        return ret;
    }
    
    // Validate CRC here if needed (omitted for brevity)

    *out_handle = handle;
    return ESP_OK;
}

static esp_err_t ms5837_get_adc(ms5837_handle_t *handle, uint8_t cmd, uint32_t *result) {
    uint8_t buffer[3];
    
    // Start conversion
    ESP_RETURN_ON_ERROR(i2c_master_transmit(handle->i2c_dev, &cmd, 1, 1000), TAG, "Failed to send conversion command");
    
    // Wait for conversion (20ms for OSR 8192)
    vTaskDelay(pdMS_TO_TICKS(20));
    
    // Read ADC
    uint8_t read_cmd = MS5837_CMD_ADC_READ;
    ESP_RETURN_ON_ERROR(i2c_master_transmit_receive(handle->i2c_dev, &read_cmd, 1, buffer, 3, 1000), TAG, "Failed to read ADC");
    
    *result = ((uint32_t)buffer[0] << 16) | ((uint32_t)buffer[1] << 8) | buffer[2];
    return ESP_OK;
}

esp_err_t ms5837_read(ms5837_handle_t *handle, ms5837_data_t *out_data) {
    uint32_t D1, D2;
    
    ESP_RETURN_ON_ERROR(ms5837_get_adc(handle, MS5837_CMD_CONVERT_D1_8192, &D1), TAG, "Failed to read pressure raw");
    ESP_RETURN_ON_ERROR(ms5837_get_adc(handle, MS5837_CMD_CONVERT_D2_8192, &D2), TAG, "Failed to read temp raw");

    // Calculation per datasheet for MS5837-30BA
    int32_t dT;
    int32_t TEMP;
    int64_t OFF, SENS;
    int32_t P;
    
    // First Order
    dT = D2 - (uint32_t)handle->C[5] * 256;
    TEMP = 2000 + ((int64_t)dT * handle->C[6]) / 8388608;
    
    OFF = (int64_t)handle->C[2] * 65536 + ((int64_t)handle->C[4] * dT) / 128;
    SENS = (int64_t)handle->C[1] * 32768 + ((int64_t)handle->C[3] * dT) / 256;
    
    // Second Order
    if (TEMP < 2000) {
        int64_t Ti = (3 * (int64_t)dT * (int64_t)dT) / 8589934592LL;
        int64_t OFFi = (3 * (int64_t)(TEMP - 2000) * (int64_t)(TEMP - 2000)) / 2;
        int64_t SENSi = (5 * (int64_t)(TEMP - 2000) * (int64_t)(TEMP - 2000)) / 8;
        
        if (TEMP < -1500) {
            OFFi = OFFi + 7 * (int64_t)(TEMP + 1500) * (int64_t)(TEMP + 1500);
            SENSi = SENSi + 4 * (int64_t)(TEMP + 1500) * (int64_t)(TEMP + 1500);
        }
        
        TEMP = TEMP - Ti;
        OFF = OFF - OFFi;
        SENS = SENS - SENSi;
    }
    
    P = (D1 * SENS / 2097152 - OFF) / 8192;
    
    out_data->pressure_mbar = P / 10.0f;
    out_data->temperature_c = TEMP / 100.0f;
    
    return ESP_OK;
}
