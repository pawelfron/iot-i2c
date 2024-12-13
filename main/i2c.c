#include <esp_rom_sys.h>

#include "i2c.h"

int read_temp(i2c_device_t *i2c, int *result) {
    printf("Odczyt temperatury\n");
    i2c_start(&i2c);
    if (!i2c_write_byte(&i2c, (0x40 << 1) | 0x0)) {
        printf("Brak ACK\n");
        i2c_stop(&i2c);
        return 1;
    }
    printf("Odczyt temperatury: wysłano adres i polecenie pisania\n");

    if (!i2c_write_byte(&i2c, 0xE3)) {
        printf("Brak ACK\n");
        i2c_stop(&i2c);
        return 1;
    }
    printf("Odczyt temperatury: wysłano polecenie odczytu temperatury\n");

    if (!i2c_write_byte(&i2c, (0x40 << 1) | 0x1)) {
        printf("Brak ACK\n");
        i2c_stop(&i2c);
        return 1;
    }
    printf("Odczyt temperatury: wysłano odres i polecenie czytania\n");
    uint8_t received_data = i2c_read_byte(&i2c, false);
    uint8_t received_data2 = i2c_read_byte(&i2c, false);
    uint8_t received_data3 = i2c_read_byte(&i2c, true);
    i2c_stop(&i2c);
    *result = (received_data << 6) | (received_data2 >> 2);
    return 0;
}

int read_hum(i2c_device_t *i2c, int *result) {
    printf("Próba pisania\n");
    i2c_start(&i2c);
    if (!i2c_write_byte(&i2c, (0x40 << 1) | 0x0)) {
        printf("Brak ACK\n");
        i2c_stop(&i2c);
        return 1;
    }
    if (!i2c_write_byte(&i2c, 0xE3)) {
        printf("Brak ACK, komenda\n");
        i2c_stop(&i2c);
        return 1;
    }

    printf("Próba czytania\n");
    i2c_start(&i2c);
    if (!i2c_write_byte(&i2c, (0x40 << 1) | 0x1)) {
        printf("Brak ACK\n");
        i2c_stop(&i2c);
        return 1;
    }
    uint8_t received_data = i2c_read_byte(&i2c, false);
    uint8_t received_data2 = i2c_read_byte(&i2c, false);
    uint8_t received_data3 = i2c_read_byte(&i2c, true);
    i2c_stop(&i2c);
    *result = (received_data << 6) | (received_data2 >> 2);
    return 0;
}

static void i2c_delay() {
    esp_rom_delay_us(I2C_DELAY_US);
}

static void i2c_set_sda(i2c_device_t *i2c, bool level) {
    gpio_set_level(i2c->sda_pin, level);
}

static bool i2c_get_sda(i2c_device_t *i2c) {
    return gpio_get_level(i2c->sda_pin);
}

static void i2c_set_scl(i2c_device_t *i2c, bool level) {
    gpio_set_level(i2c->scl_pin, level);
}

void i2c_init(i2c_device_t *i2c, gpio_num_t sda, gpio_num_t scl, const char *name) {
    i2c->sda_pin = sda;
    i2c->scl_pin = scl;
    i2c->name = name;

    gpio_set_direction(sda, GPIO_MODE_INPUT_OUTPUT_OD);
    gpio_set_pull_mode(sda, GPIO_PULLUP_ONLY);

    gpio_set_direction(scl, GPIO_MODE_INPUT_OUTPUT_OD);
    gpio_set_pull_mode(scl, GPIO_PULLUP_ONLY);

    printf("Urządzenie %s zainicjalizowane; SDA: %d, SCL: %d\n", name, sda, scl);
}

void i2c_start(i2c_device_t *i2c) {
    i2c_set_sda(i2c, 1);
    i2c_set_scl(i2c, 1);
    i2c_delay();
    i2c_set_sda(i2c, 0);
    i2c_delay();
    i2c_set_scl(i2c, 0);
}

void i2c_stop(i2c_device_t *i2c) {
    i2c_set_sda(i2c, 0);
    i2c_set_scl(i2c, 1);
    i2c_delay();
    i2c_set_sda(i2c, 1);
    i2c_delay();
}

bool i2c_write_byte(i2c_device_t *i2c, uint8_t byte) {
    for (int i = 0; i < 8; i++) {
        i2c_set_sda(i2c, (byte & 0x80) != 0);
        byte <<= 1;
        i2c_delay();

        i2c_set_scl(i2c, 1);
        i2c_delay();
        i2c_set_scl(i2c, 0);
        i2c_delay();
    }

    gpio_set_direction(i2c->sda_pin, GPIO_MODE_INPUT);
    i2c_delay();

    i2c_set_scl(i2c, 1);
    i2c_delay();
    bool ack = !i2c_get_sda(i2c);
    i2c_set_scl(i2c, 0);
    i2c_delay();

    gpio_set_direction(i2c->sda_pin, GPIO_MODE_INPUT_OUTPUT_OD);

    return ack;
}

uint8_t i2c_read_byte(i2c_device_t *i2c, bool ack) {
    uint8_t byte = 0;

    gpio_set_direction(i2c->sda_pin, GPIO_MODE_INPUT);

    for (int i = 0; i < 8; i++) {
        byte <<= 1;

        i2c_set_scl(i2c, 1);
        i2c_delay();

        if (i2c_get_sda(i2c)) {
            byte |= 0x01;
        }

        i2c_set_scl(i2c, 0);
        i2c_delay();
    }

    gpio_set_direction(i2c->sda_pin, GPIO_MODE_INPUT_OUTPUT_OD);

    i2c_set_sda(i2c, !ack);
    i2c_delay();

    i2c_set_scl(i2c, 1);
    i2c_delay();
    i2c_set_scl(i2c, 0);
    i2c_delay();

    return byte;
}
