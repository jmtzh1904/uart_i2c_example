#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c_master.h"
#include "esp_log.h"

#define I2C_SCL GPIO_NUM_2  
#define I2C_SDA GPIO_NUM_3
#define BH1750_ADDR 0X23 

// Comandos del BH1750
#define BH1750_CMD_POWER_ON     0x01    // Encender
#define BH1750_CMD_RESET         0x07    // Resetear registro de datos
#define BH1750_CMD_CONT_HR       0x10    // Modo continuo alta resolución (1lx, 120ms)
#define BH1750_CMD_CONT_HR2      0x11    // Modo continuo alta resolución 2 (0.5lx, 120ms)
#define BH1750_CMD_CONT_LR       0x13    // Modo continuo baja resolución (4lx, 16ms)
#define BH1750_CMD_ONETIME_HR    0x20    // Una medición alta resolución (1lx, 120ms)
#define BH1750_CMD_ONETIME_HR2   0x21    // Una medición alta resolución 2 (0.5lx, 120ms)
#define BH1750_CMD_ONETIME_LR    0x23    // Una medición baja resolución (4lx, 16ms)

static i2c_master_bus_handle_t bus_handle;
static i2c_master_dev_handle_t dev_handle;

static const char *TAG = "UART_TEST";

void i2c_init(void);
esp_err_t send_cmd(uint8_t cmd, i2c_master_dev_handle_t* dev_handle);
esp_err_t send_receive_cmd(uint8_t cmd, i2c_master_dev_handle_t* dev_handle, float* dato);


void app_main(void){
    
    ESP_LOGI(TAG, "Inicializando I2C");
    i2c_init();

    ESP_ERROR_CHECK(send_cmd(BH1750_CMD_POWER_ON, &dev_handle));

    float lux;
    while(1){
        ESP_ERROR_CHECK(send_receive_cmd(BH1750_CMD_CONT_HR, &dev_handle, &lux));
        ESP_LOGI(TAG, "EL nivel de lux: %.2f", lux);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}


void i2c_init(void){

    i2c_master_bus_config_t i2c_mst_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = -1,
        .scl_io_num = I2C_SCL,
        .sda_io_num = I2C_SDA,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_mst_config, &bus_handle));

    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_7,
        .device_address = BH1750_ADDR,
        .scl_speed_hz = 100000,
        .scl_wait_us = 0,
    };
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_cfg, &dev_handle));
}

esp_err_t send_cmd(uint8_t cmd, i2c_master_dev_handle_t* dev_handle){
    return i2c_master_transmit(*dev_handle, &cmd, 1, -1);
}

esp_err_t send_receive_cmd(const uint8_t cmd, i2c_master_dev_handle_t* dev_handle, float* dato){

    esp_err_t ret;
    uint8_t buffer[2];
    
    ret = i2c_master_transmit_receive(*dev_handle, &cmd, 1, buffer, 2, -1);
    if (ret == ESP_OK){
        uint16_t raw_value = (buffer[0] << 8) | buffer[1];
        *dato = raw_value / 1.2f;
    }
    ESP_LOGI(TAG, "El primer byte es %d y el segundo %d", buffer[0], buffer[1]);
    return ret;
}