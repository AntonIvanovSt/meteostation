// scd41_api.h
#ifndef SCD41_H_
#define SCD41_H_

#include "driver/i2c.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "scd41.h"

#define TEST_I2C_PORT I2C_NUM_0
#define I2C_MASTER_SCL_IO 22
#define I2C_MASTER_SDA_IO 21

#define SCD41_READY BIT0

extern EventGroupHandle_t s_scd41_event_group;

typedef struct {
    float co2;
    float temperature;
    float humidity;
} scd41_custom_t;

void init_scd41(void);
esp_err_t read_scd41(scd41_custom_t *out);
void scd41_task(void *pvParameters);

#endif // !SCD41_H_
