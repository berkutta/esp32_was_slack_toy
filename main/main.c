
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event_loop.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "mqtt_client.h"
#include "cJSON.h"

#include "config.h"

#include "servo.h"

static const char *TAG = "mqtt";

static EventGroupHandle_t wifi_event_group;
const static int CONNECTED_BIT = BIT0;

static esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event)
{
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");

            msg_id = esp_mqtt_client_subscribe(client, "/gadget/rat/control", 1);
            ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            break;

        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, 0, 0);
            ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT_EVENT_DATA");
            printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
            printf("DATA=%.*s\r\n", event->data_len, event->data);

            if(strncmp(event->topic, "/gadget/rat/control", event->topic_len) == 0) {
                const cJSON *mode = NULL;

                cJSON *dataset = cJSON_Parse(event->data);

                if(dataset != NULL) {
                    mode = cJSON_GetObjectItemCaseSensitive(dataset, "mode");

                    if (cJSON_IsString(mode) && (mode->valuestring != NULL)) {
                        if(strcmp(mode->valuestring, "run") == 0) {
                            const cJSON *amount = NULL;
                            const cJSON *minimum_amplitude = NULL;
                            const cJSON *maximum_amplitude = NULL;
                            const cJSON *delay = NULL;

                            amount = cJSON_GetObjectItemCaseSensitive(dataset, "amount");
                            minimum_amplitude = cJSON_GetObjectItemCaseSensitive(dataset, "maximum");
                            maximum_amplitude = cJSON_GetObjectItemCaseSensitive(dataset, "minimum");
                            delay = cJSON_GetObjectItemCaseSensitive(dataset, "delay");

                            if( cJSON_IsNumber(amount) && 
                                cJSON_IsNumber(minimum_amplitude) && 
                                cJSON_IsNumber(maximum_amplitude) &&
                                cJSON_IsNumber(delay) ) {
                                    cJSON *ok_answer = cJSON_CreateObject();

                                    cJSON_AddStringToObject(ok_answer, "status", "OK");
                                    
                                    msg_id = esp_mqtt_client_publish(client, "/gadget/rat/feedback", cJSON_Print(ok_answer), 0, 0, 0);
                                    ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
                                    
                                    servo_run(amount->valueint, minimum_amplitude->valueint, maximum_amplitude->valueint, delay->valueint);
                            } else {
                                cJSON *nok_answer = cJSON_CreateObject();

                                cJSON_AddStringToObject(nok_answer, "status", "Error");
                                
                                msg_id = esp_mqtt_client_publish(client, "/gadget/rat/feedback", cJSON_Print(nok_answer), 0, 0, 0);
                                ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
                            }

                            
                        }
                    }
                }

                // if(strncmp(event->data, "run", event->data_len) == 0)    
            }
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            break;
    }
    return ESP_OK;
}

static esp_err_t wifi_event_handler(void *ctx, system_event_t *event)
{
    switch (event->event_id) {
        case SYSTEM_EVENT_STA_START:
            esp_wifi_connect();
            break;
        case SYSTEM_EVENT_STA_GOT_IP:
            xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);

            break;
        case SYSTEM_EVENT_STA_DISCONNECTED:
            esp_wifi_connect();
            xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
            break;
        default:
            break;
    }
    return ESP_OK;
}

static void mqtt_init(void)
{
    const esp_mqtt_client_config_t mqtt_cfg = {
        .uri = mqtt_address,
        .event_handle = mqtt_event_handler,
        .username = mqtt_username,
        .password = mqtt_password
    };

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_start(client);
}

static void wifi_init(void)
{
    tcpip_adapter_init();
    wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_event_loop_init(wifi_event_handler, NULL));
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = wifi_ssid,
            .password = wifi_password,
            .scan_method = true
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    //ESP_LOGI(TAG, "start the WIFI SSID:[%s] password:[%s]", CONFIG_WIFI_SSID, "******");
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_LOGI(TAG, "Waiting for wifi");
    xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT, false, true, portMAX_DELAY);
}

void app_main(void)
{
    nvs_flash_init();
    servo_init();
    wifi_init();
    mqtt_init();
}

