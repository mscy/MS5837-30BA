/*
 * Copyright (c) 2026 cysoft
 * All rights reserved.
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c_master.h"
#include "esp_log.h"
#include "ms5837.h"

// Define I2C pins. Verify these match your board!
#define I2C_MASTER_SDA_IO 6
#define I2C_MASTER_SCL_IO 7
#define I2C_MASTER_FREQ_HZ 100000

static const char *TAG = "MAIN";

void i2c_scan(i2c_master_bus_handle_t bus_handle) {
    ESP_LOGI(TAG, "Starting I2C scan...");
    int found_count = 0;
    for (uint8_t addr = 0x08; addr < 0x78; addr++) {
        esp_err_t ret = i2c_master_probe(bus_handle, addr, 100);
        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "Found device at: 0x%02x", addr);
            found_count++;
        }
    }
    ESP_LOGI(TAG, "I2C scan complete. Found %d devices.", found_count);
}

void app_main(void)
{
    // Allow power to stabilize
    vTaskDelay(pdMS_TO_TICKS(2000));

    esp_log_level_set(TAG, ESP_LOG_DEBUG);
    ESP_LOGI(TAG, "Starting MS5837 Driver Test");

    // Initialize I2C Master Bus
    i2c_master_bus_config_t i2c_bus_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = -1, // Auto-select path
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    
    i2c_master_bus_handle_t bus_handle;
    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_bus_config, &bus_handle));

    // Run I2C scan
    // i2c_scan(bus_handle);

    // Initialize MS5837 Sensor
    ms5837_handle_t *sensor_handle = NULL;
    ESP_LOGI(TAG, "Initializing sensor at address 0x76...");
    esp_err_t ret = ms5837_init(bus_handle, 0x76, &sensor_handle);
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize MS5837: %s", esp_err_to_name(ret));
        return;
    }
    
    ESP_LOGI(TAG, "Sensor initialized successfully.");

    ms5837_data_t data;
    while (1) {
        ret = ms5837_read(sensor_handle, &data);
        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "Pressure: %.4f Bar, Temperature: %.2f C", data.pressure_mbar / 1000.0f, data.temperature_c);
        } else {
            ESP_LOGE(TAG, "Failed to read sensor: %s", esp_err_to_name(ret));
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
