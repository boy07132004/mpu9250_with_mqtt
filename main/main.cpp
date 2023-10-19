/* SPI Master Half Duplex EEPROM example.

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/queue.h"
#include "esp_timer.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "soc/gpio_periph.h"
#include "hal/gpio_hal.h"
#include "sdkconfig.h"
#include "driver/gpio.h"
#include <bits/stdc++.h>
#include "esp_log.h"
#include "zmlib.h"
#include "interface/zm_wifi.h"
#include "interface/SPIbus.hpp"

#define PIN_NUM_CLK 7
#define PIN_NUM_MISO 8
#define PIN_NUM_MOSI 9

#define FREQ_TO_INTERVAL(freq) ((1.0 / (freq)) * 1000000)
#define USER_LED GPIO_NUM_21
#define LED_ON 0
#define LED_OFF 1

extern const char *TAG = "main";

TaskHandle_t myTaskHandle = NULL;
TaskHandle_t myTaskHandle2 = NULL;

extern QueueHandle_t task_from_mqtt = xQueueCreate(10, sizeof(struct task));
extern QueueHandle_t task_to_transmit = xQueueCreate(10, sizeof(struct task));

uint32_t sensor_curr_idx = 0;
std::string PUBLISH_MSG;
float *x, *y, *z;

void sensor_task(void *arg)
{
   struct task received_task;
   struct task send_task;

   while (1)
   {
      if (xQueueReceive(task_from_mqtt, &received_task, portMAX_DELAY) == pdPASS)
      {
         // printf("Receive: \n-----------\nFREQ: %d\nSEC: %f\nNAME:%s\n", received_task.freq, received_task.seconds, received_task.topic);
         memcpy(&send_task, &received_task, sizeof(received_task));

         gpio_set_level(USER_LED, LED_OFF);
         collect_3axis_accel_data(x, y, z, FREQ_TO_INTERVAL(received_task.freq), (long)received_task.freq * received_task.seconds);
         xQueueSend(task_to_transmit, &send_task, portMAX_DELAY);
         gpio_set_level(USER_LED, LED_ON);
      }
   }
}

void transmit_task(void *arg)
{
   wifi_config_t wifi_config = {
       .sta = {
           .ssid = CONFIG_ESP_WIFI_SSID,
           .password = CONFIG_ESP_WIFI_PASSWORD,
           .threshold = {
               .authmode = ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD}},
   };
   struct task received_task;
   uint32_t size;
   char topic[100];
   uint32_t transmit_curr_idx = 0;
   uint64_t start_time;

   printf("Connect to wifi\n");
   wifi_init_sta(&wifi_config);
   delay_ms(100);
   printf("Connect to mqtt\n");
   gpio_set_level(USER_LED, LED_ON);
   mqtt_app_start();
   delay_ms(100);

   char buf[22];

   while (1)
   {
      if (xQueueReceive(task_to_transmit, &received_task, portMAX_DELAY) == pdPASS)
      {
         size = received_task.seconds * (uint32_t)received_task.freq;
         snprintf(topic, sizeof(topic), "%s", received_task.topic);
         printf("Start publishing data to topic: %s", received_task.topic);
         transmit_curr_idx = 0;

         while (transmit_curr_idx < size)
         {
            if (transmit_curr_idx + 1000 >= size)
            {
               for (; transmit_curr_idx < size; transmit_curr_idx++)
               {
                  snprintf(buf, sizeof(buf), "%.3f,%.3f,%.3f\n", x[transmit_curr_idx], y[transmit_curr_idx], z[transmit_curr_idx]);
                  PUBLISH_MSG.append(buf);
                  memset(buf, 0, sizeof(buf));
               }
               zm_publish(topic, PUBLISH_MSG.c_str());
               zm_publish(topic, "end");
               PUBLISH_MSG.clear();
               printf("Finished publish.\n");

               break;
            }
            else
            {
               for (int i = 0; i < 1000; i++)
               {
                  snprintf(buf, sizeof(buf), "%.3f,%.3f,%.3f\n", x[transmit_curr_idx + i], y[transmit_curr_idx + i], z[transmit_curr_idx + i]);
                  PUBLISH_MSG.append(buf);
                  memset(buf, 0, sizeof(buf));
               }
               transmit_curr_idx += 1000;
               zm_publish(topic, PUBLISH_MSG.c_str());
               PUBLISH_MSG.clear();
            }
         }
      }
   }
}

extern "C" void app_main(void)
{
   gpio_set_direction(USER_LED, GPIO_MODE_OUTPUT);

   x = (float *)malloc(sizeof(float) * MAX_DATA_SIZE);
   y = (float *)malloc(sizeof(float) * MAX_DATA_SIZE);
   z = (float *)malloc(sizeof(float) * MAX_DATA_SIZE);

   xTaskCreatePinnedToCore(sensor_task, "sensor_task", 4096, NULL, 10, &myTaskHandle, 0);
   xTaskCreatePinnedToCore(transmit_task, "transmit_task", 4096, NULL, 10, &myTaskHandle2, 0);
}