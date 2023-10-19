#ifndef _ZMLIB_H_
#define _ZMLIB_H_

#define MAX_DATA_SIZE (500000)

#include "esp_err.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "driver_mpu9250_basic.h"
#include "esp_log.h"
#include "mqtt_client.h"
extern const char *TAG;
extern QueueHandle_t task_from_mqtt;

struct task
{
    char topic[100];
    uint16_t freq;
    float seconds;
};

#ifdef __cplusplus
extern "C"
{
#endif
    esp_err_t collect_3axis_accel_data(float *x, float *y, float *z, int interval, uint32_t number_of_samples);
    void delay_ms(uint32_t ms);
    void mqtt_app_start(void);
    void zm_publish(const char *topic, const char *msg);
    static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);
    static void log_error_if_nonzero(const char *message, int error_code);

#ifdef __cplusplus
}
#endif

#endif // _ZMLIB_H_