#include <freertos/FreeRTOS.h>

#include "i2c.h"

#define I2C_SDA_PIN  GPIO_NUM_18
#define I2C_SCL_PIN  GPIO_NUM_19

typedef struct {
    uint16_t dig_T1;
    int16_t dig_T2;
    int16_t dig_T3;
    uint16_t dig_P1;
    int16_t dig_P2;
    int16_t dig_P3;
    int16_t dig_P4;
    int16_t dig_P5;
    int16_t dig_P6;
    int16_t dig_P7;
    int16_t dig_P8;
    int16_t dig_P9;
} calib_t;

calib_t calib;

int get_calibration_params(i2c_device_t i2c) {
    printf("Odczyt kalibracyjny: wysłanie adresu\n");
    i2c_start(&i2c);
    if (!i2c_write_byte(&i2c, (0x76 << 1) | 0x0)) {
        printf("Odczyt kalibracyjny: Brak ACK\n");
        i2c_stop(&i2c);
        return 1;
    }
    printf("Odczyt kalibracyjny: otrzymano ACK\n");

    printf("Odczyt temperatury: wysłanie adresu rejestru 88\n");
    if (!i2c_write_byte(&i2c, 0x88)) {
        printf("Odczyt kalibracyjny: Brak ACK\n");
        i2c_stop(&i2c);
        return 1;
    }
    printf("Odczyt kalibracyjny: otrzymano ACK\n");

    // Właściwy odczyt
    printf("Odczyt kalibracyjny: ponowne wysłanie adresu\n");
    i2c_start(&i2c);
    if (!i2c_write_byte(&i2c, (0x76 << 1) | 0x1)) {
        printf("Odczyt kalibracyjny: Brak ACK\n");
        i2c_stop(&i2c);
        return 1;
    }
    printf("Odczyt kalibracyjny: otrzymano ACK\n");

    printf("Odczyt kalibracyjny: czytanie wartości\n");
    calib.dig_T1 = i2c_read_byte(&i2c, true);
    calib.dig_T1 |= i2c_read_byte(&i2c, true) << 8;

    calib.dig_T2 = i2c_read_byte(&i2c, true);
    calib.dig_T2 |= i2c_read_byte(&i2c, true) << 8;

    calib.dig_T3 = i2c_read_byte(&i2c, true);
    calib.dig_T3 |= i2c_read_byte(&i2c, true) << 8;

    calib.dig_P1 = i2c_read_byte(&i2c, true);
    calib.dig_P1 |= i2c_read_byte(&i2c, true) << 8;

    calib.dig_P2 = i2c_read_byte(&i2c, true);
    calib.dig_P2 |= i2c_read_byte(&i2c, true) << 8;

    calib.dig_P3 = i2c_read_byte(&i2c, true);
    calib.dig_P3 |= i2c_read_byte(&i2c, true) << 8;

    calib.dig_P4 = i2c_read_byte(&i2c, true);
    calib.dig_P4 |= i2c_read_byte(&i2c, true) << 8;

    calib.dig_P5 = i2c_read_byte(&i2c, true);
    calib.dig_P5 |= i2c_read_byte(&i2c, true) << 8;

    calib.dig_P6 = i2c_read_byte(&i2c, true);
    calib.dig_P6 |= i2c_read_byte(&i2c, true) << 8;

    calib.dig_P7 = i2c_read_byte(&i2c, true);
    calib.dig_P7 |= i2c_read_byte(&i2c, true) << 8;

    calib.dig_P8 = i2c_read_byte(&i2c, true);
    calib.dig_P8 |= i2c_read_byte(&i2c, true) << 8;

    calib.dig_P9 = i2c_read_byte(&i2c, true);
    calib.dig_P9 |= i2c_read_byte(&i2c, false) << 8;
    
    i2c_stop(&i2c);
    printf("T: %d %d %d\n", calib.dig_T1, calib.dig_T2, calib.dig_T3);
    printf("P: %d %d %d %d %d %d %d %d %d\n",
            calib.dig_P1, calib.dig_P2, calib.dig_P3,
            calib.dig_P4, calib.dig_P5, calib.dig_P6,
            calib.dig_P7, calib.dig_P8, calib.dig_P9);
    return 0;
}

int trigger_measurment(i2c_device_t i2c) {
    printf("Wywoływanie pomiaru\n");
    i2c_start(&i2c);
    if (!i2c_write_byte(&i2c, (0x76 << 1) | 0x0)) {
        printf("Wywoływanie pomiaru: Brak ACK\n");
        i2c_stop(&i2c);
        return 1;
    }
    printf("Wysyłanie adresu rejestru\n");
    if (!i2c_write_byte(&i2c, 0xF4)) {
        printf("Wywoływanie pomiaru: Brak ACK\n");
        i2c_stop(&i2c);
        return 1;
    }
    printf("Wysyłanie wartości 0x25\n");
    if (!i2c_write_byte(&i2c, 0x25)) {
        printf("Wywoływanie pomiaru: Brak ACK\n");
        i2c_stop(&i2c);
        return 1;
    }
    i2c_stop(&i2c);

    bool measurement_in_progress = true;
    while (measurement_in_progress) {
        i2c_start(&i2c);
        if (!i2c_write_byte(&i2c, (0x76 << 1) | 0x0)) {
            printf("Odczyt statusu: Brak ACK\n");
            i2c_stop(&i2c);
            return 1;
        }
        if (!i2c_write_byte(&i2c, 0xF3)) {
            printf("Odczyt statusu: Brak ACK\n");
            i2c_stop(&i2c);
            return 1;
        }

        i2c_start(&i2c);
        if (!i2c_write_byte(&i2c, (0x76 << 1) | 0x1)) {
            printf("Odczyt statusu: Brak ACK\n");
            i2c_stop(&i2c);
            return 1;
        }
        uint8_t data_F3 = i2c_read_byte(&i2c, false);
        i2c_stop(&i2c);
        printf("Status %x\n", data_F3);
        if ((data_F3 & 0b00001000) == 0 && (data_F3 & 0b00000001) == 0) {
            measurement_in_progress = false;
        }

        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    return 0;
}

int read_temp(i2c_device_t i2c, int32_t *result) {
    printf("Odczyt temperatury: wysłanie adresu\n");
    i2c_start(&i2c);
    if (!i2c_write_byte(&i2c, (0x76 << 1) | 0x0)) {
        printf("Odczyt temperatury: Brak ACK\n");
        i2c_stop(&i2c);
        return 1;
    }
    printf("Odczyt temperatury: otrzymano ACK\n");

    printf("Odczyt temperatury: wysłanie adresu rejestru F6\n");
    if (!i2c_write_byte(&i2c, 0xF6)) {
        printf("Odczyt temperatury: Brak ACK\n");
        i2c_stop(&i2c);
        return 1;
    }
    printf("Odczyt temperatury: otrzymano ACK\n");

    // Właściwy odczyt
    printf("Odczyt temperatury: ponowne wysłanie adresu\n");
    i2c_start(&i2c);
    if (!i2c_write_byte(&i2c, (0x76 << 1) | 0x1)) {
        printf("Odczyt temperatury: Brak ACK\n");
        i2c_stop(&i2c);
        return 1;
    }
    printf("Odczyt temperatury: otrzymano ACK\n");

    printf("Odczyt temperatury: czytanie wartości\n");
    uint8_t data_F6 = i2c_read_byte(&i2c, true);
    uint8_t data_F7 = i2c_read_byte(&i2c, true);
    uint8_t data_F8 = i2c_read_byte(&i2c, true);
    uint8_t data_F9 = i2c_read_byte(&i2c, true);
    uint8_t data_FA = i2c_read_byte(&i2c, true);
    uint8_t data_FB = i2c_read_byte(&i2c, true);
    uint8_t data_FC = i2c_read_byte(&i2c, false);
    i2c_stop(&i2c);
    printf("%x %x %x %x %x %x %x\n", data_F6, data_F7, data_F8, data_F9, data_FA, data_FB, data_FC);
    *result = (data_FA << 12) | (data_FB << 4) | (data_FC >> 4);
    return 0;
}

int read_id(i2c_device_t i2c, int *result) {
    printf("Odczyt id: wysłanie adresu\n");
    i2c_start(&i2c);
    if (!i2c_write_byte(&i2c, (0x76 << 1) | 0x0)) {
        printf("Odczyt id: Brak ACK\n");
        i2c_stop(&i2c);
        return 1;
    }
    printf("Odczyt id: otrzymano ACK\n");

    printf("Odczyt id: wysłanie adresu rejestru D0\n");
    if (!i2c_write_byte(&i2c, 0xD0)) {
        printf("Odczyt id: Brak ACK\n");
        i2c_stop(&i2c);
        return 1;
    }
    printf("Odczyt id: otrzymano ACK\n");

    // Właściwy odczyt
    printf("Odczyt id: ponowne wysłanie adresu\n");
    i2c_start(&i2c);
    if (!i2c_write_byte(&i2c, (0x76 << 1) | 0x1)) {
        printf("Odczyt id: Brak ACK\n");
        i2c_stop(&i2c);
        return 1;
    }
    printf("Odczyt id: otrzymano ACK\n");

    printf("Odczyt id: czytanie wartości\n");
    uint8_t data_id = i2c_read_byte(&i2c, false);
    i2c_stop(&i2c);
    printf("%x\n", data_id);
    return 1;
}

// YOINK

int32_t t_fine;
int32_t bmp280_compensate_T_int32(int32_t adc_T) {
    int32_t var1, var2, T;
    var1 = ((((adc_T>>3) - ((int32_t) calib.dig_T1<<1))) * ((int32_t) calib.dig_T2)) >> 11;
    var2 = (((((adc_T>>4) - ((int32_t) calib.dig_T1)) * ((adc_T>>4) - ((int32_t) calib.dig_T1))) >> 12) * ((int32_t) calib.dig_T3)) >> 14;
    t_fine = var1 + var2;
    T = (t_fine * 5 + 128) >> 8;
    return T;
}

// ----

void app_main() {
    i2c_device_t i2c;
    i2c_init(&i2c, I2C_SDA_PIN, I2C_SCL_PIN, "I2C_TEST");
    
    int32_t result;
    get_calibration_params(i2c);
    trigger_measurment(i2c);
    read_temp(i2c, &result);
    printf("Raw temperature: %ld\n", result);
    int32_t temp = bmp280_compensate_T_int32(result);
    printf("Compensated temperature: %ld\n", temp);
}
