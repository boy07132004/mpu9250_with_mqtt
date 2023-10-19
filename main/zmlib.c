#include "zmlib.h"

static esp_mqtt_client_handle_t client;

esp_err_t collect_3axis_accel_data(float *x, float *y, float *z, int interval, uint32_t number_of_samples)
{
    float g[3];
    mpu9250_address_t addr = MPU9250_ADDRESS_AD0_LOW;
    if (mpu9250_basic_init(MPU9250_INTERFACE_SPI, addr) != 0)
    {
        return ESP_FAIL;
    }
    printf("Start monitoring accelerator.\n");
    uint64_t last_time = esp_timer_get_time();
    uint64_t start_time = esp_timer_get_time();

    for (uint32_t i = 0; i < number_of_samples; i++)
    {
        while (esp_timer_get_time() - last_time < interval)
            ;
        last_time = esp_timer_get_time();
        if (mpu9250_zm_read(g) != 0)
        {
            (void)mpu9250_basic_deinit();

            return ESP_FAIL;
        }
        x[i] = g[0];
        y[i] = g[1];
        z[i] = g[2];
    }
    mpu9250_interface_debug_print("%ld data, cost  %f sec\n", number_of_samples, (esp_timer_get_time() - start_time) / 1000000.0);
    mpu9250_basic_deinit();
    return ESP_OK;
}

void delay_ms(uint32_t ms)
{
    vTaskDelay(ms / portTICK_PERIOD_MS);
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32 "", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    switch ((esp_mqtt_event_id_t)event_id)
    {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        // msg_id = esp_mqtt_client_publish(client, "/topic/qos1", "data_3", 0, 1, 0);
        // ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);

        msg_id = esp_mqtt_client_subscribe(client, "task", 2);
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

        // msg_id = esp_mqtt_client_subscribe(client, "/topic/qos1", 1);
        // ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

        // msg_id = esp_mqtt_client_unsubscribe(client, "/topic/qos1");
        // ESP_LOGI(TAG, "sent unsubscribe successful, msg_id=%d", msg_id);
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;

    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");

        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);

        if (strncmp(event->topic, "task", event->topic_len) == 0)
        {
            char buffer[3][100];
            int buffer_idx = 0;
            int char_idx = 0;
            for (int i = 0; i < event->data_len; i++)
            {
                if ((event->data[i] == ',') && buffer_idx < 2)
                {
                    buffer[buffer_idx++][char_idx] = '\0';
                    char_idx = 0;
                }
                else
                {
                    buffer[buffer_idx][char_idx++] = event->data[i];
                }
            }
            buffer[buffer_idx][char_idx] = '\0';
            if (buffer_idx != 2)
                goto MQTT_TASK_ERROR1;

            uint16_t freq = atoi(buffer[0]);
            float sec = atof(buffer[1]);

            if (freq <= 0 || freq > 4000 || sec <= 0 || (freq * sec) >= (MAX_DATA_SIZE))
            {
                goto MQTT_TASK_ERROR2;
            }

            struct task msg_to_queue = {
                .freq = freq,
                .seconds = sec};
            snprintf(msg_to_queue.topic, sizeof(buffer[2]), "%s", buffer[2]);
            xQueueSend(task_from_mqtt, &msg_to_queue, portMAX_DELAY);
            // printf("%d, %f, %s\n", freq, sec, buffer[2]);
        }
        else
        {
            printf("%s\n", event->topic);
        }
        break;
    MQTT_TASK_ERROR1:
        ESP_LOGE(TAG, "MQTT TASK IDX ERROR.");
        break;
    MQTT_TASK_ERROR2:
        ESP_LOGE(TAG, "MQTT TASK PARAMETER ERROR.");
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT)
        {
            log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno", event->error_handle->esp_transport_sock_errno);
            ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
        }
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}

void mqtt_app_start(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = CONFIG_BROKER_URL,
    };
    client = esp_mqtt_client_init(&mqtt_cfg);
    /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
}

void zm_publish(const char *topic, const char *msg)
{
    int msg_id = esp_mqtt_client_publish(client, topic, msg, 0, 2, 0);
    ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
}

static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0)
    {
        ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}