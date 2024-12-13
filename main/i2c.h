#pragma once

#include "driver/gpio.h"

#define I2C_FREQ_HZ    10000    // 100 kHz I2C frequency
#define I2C_DELAY_US   (1000000 / I2C_FREQ_HZ / 2)

typedef struct {
    gpio_num_t sda_pin;
    gpio_num_t scl_pin;
    const char *name;
} i2c_device_t;

void i2c_init(i2c_device_t *i2c, gpio_num_t sda, gpio_num_t scl, const char *name);
void i2c_start(i2c_device_t *i2c);
void i2c_stop(i2c_device_t *i2c);
bool i2c_write_byte(i2c_device_t *i2c, uint8_t byte);
uint8_t i2c_read_byte(i2c_device_t *i2c, bool ack);