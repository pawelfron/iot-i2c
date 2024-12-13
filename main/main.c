#include <freertos/FreeRTOS.h>

#include "i2c.h"

#define I2C_SDA_PIN  GPIO_NUM_21
#define I2C_SCL_PIN  GPIO_NUM_22
#define I2C_SLAVE_ADDR  0x40

void app_main() {
    i2c_device_t i2c;
    i2c_init(&i2c, I2C_SDA_PIN, I2C_SCL_PIN, "I2C_TEST");
    
    printf("Próba pisania\n");
    i2c_start(&i2c);
    if (!i2c_write_byte(&i2c, (I2C_SLAVE_ADDR << 1) | 0x0)) {
        printf("Brak ACK\n");
        i2c_stop(&i2c);
        return;
    }
    if (!i2c_write_byte(&i2c, 0xE3)) {
        printf("Brak ACK, komenda\n");
        i2c_stop(&i2c);
        return;
    }
    // i2c_stop(&i2c);

    // vTaskDelay(2000 / portTICK_PERIOD_MS);

    printf("Próba czytania\n");
        // i2c_start(&i2c);
    if (!i2c_write_byte(&i2c, (I2C_SLAVE_ADDR << 1) | 0x1)) {
        printf("Brak ACK\n");
        i2c_stop(&i2c);
        return;
    }
    uint8_t received_data = i2c_read_byte(&i2c, false);
    uint8_t received_data2 = i2c_read_byte(&i2c, false);
    uint8_t received_data3 = i2c_read_byte(&i2c, true);
    i2c_stop(&i2c);
    printf("Otrzymane dane: 0x%02X 0x%02X 0x%02X\n", received_data, received_data2, received_data3);
}
