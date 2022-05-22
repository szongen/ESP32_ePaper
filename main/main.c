#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_http_client.h"
#include "cJSON.h"

#include "sdkconfig.h"
#include "esp_log.h"
#include "E2213JS0C1.h"
#include "image.h"

#define EEPROM_HOST HSPI_HOST
#define PIN_NUM_MISO 18
#define PIN_NUM_MOSI 23
#define PIN_NUM_CLK 19

#define EXAMPLE_ESP_WIFI_SSID "502"
#define EXAMPLE_ESP_WIFI_PASS "RD35462959"
#define EXAMPLE_ESP_MAXIMUM_RETRY 5
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

static const char *TAG = "wifi station";

static int s_retry_num = 0;

static EventGroupHandle_t s_wifi_event_group;
static QueueHandle_t QueueHandle_sys;
spi_device_handle_t spi;

typedef struct
{
    uint8_t timestamp;
    uint8_t temperature;
    uint8_t timezone
} variable, *var;

variable *handler;
static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY)
        {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        }
        else
        {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG, "connect to the AP fail");
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

static void IRAM_ATTR spi_ready(spi_transaction_t *trans)
{
}

#define BLINK_GPIO 2
void Blink()
{
    gpio_reset_pin(BLINK_GPIO);
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
    while (1)
    {
        gpio_set_level(BLINK_GPIO, 0);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        gpio_set_level(BLINK_GPIO, 1);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

//事件回调
static esp_err_t _http_event_handle(esp_http_client_event_t *evt)
{
    uint32_t ret;
    switch (evt->event_id)
    {
    case HTTP_EVENT_ERROR: //错误事件
        ESP_LOGI(TAG, "HTTP_EVENT_ERROR");
        break;
    case HTTP_EVENT_ON_CONNECTED: //连接成功事件
        ESP_LOGI(TAG, "HTTP_EVENT_ON_CONNECTED");
        break;
    case HTTP_EVENT_HEADER_SENT: //发送头事件
        ESP_LOGI(TAG, "HTTP_EVENT_HEADER_SENT");
        break;
    case HTTP_EVENT_ON_HEADER: //接收头事件
        ESP_LOGI(TAG, "HTTP_EVENT_ON_HEADER");
        printf("%.*s", evt->data_len, (char *)evt->data);
        break;
    case HTTP_EVENT_ON_DATA: //接收数据事件
        ESP_LOGI(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
        if (!esp_http_client_is_chunked_response(evt->client))
        {
            printf("%.*s\r\n", evt->data_len, (char *)evt->data);
            ret = xQueueSend(QueueHandle_sys, evt->data, 0);
            if (ret == pdPASS)
            {
                ESP_LOGI(TAG, "HTTP_EVENT_ON_DATA xQueueSend\r\n");
            }
        }

        break;
    case HTTP_EVENT_ON_FINISH: //会话完成事件
        ESP_LOGI(TAG, "HTTP_EVENT_ON_FINISH");
        break;
    case HTTP_EVENT_DISCONNECTED: //断开事件
        ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
        break;
    }
    return ESP_OK;
}

// http client配置
esp_http_client_config_t config = {
    .method = HTTP_METHOD_POST,                                                                                          // post请求
    .url = "http://api.seniverse.com/v3/weather/now.json?key=SYpTEyjFxwWJdSuAd&location=fuzhou&language=zh-Hans&unit=c", //请求url
    .host = "api.seniverse.com",
    .skip_cert_common_name_check = true,
    .keep_alive_enable = false,
    .port = 80,
    .buffer_size = 512,
    .transport_type = HTTP_TRANSPORT_OVER_TCP,
    .event_handler = _http_event_handle, //注册时间回调
};

//测试入口
void http_client_test(void)
{
    int ret;
    char pReadBuf[512];
    esp_http_client_handle_t client = esp_http_client_init(&config); //初始化配置
    esp_err_t err = esp_http_client_perform(client);                 //执行请求

    if (err == ESP_OK)
    {
        ESP_LOGI(TAG, "Status = %d, content_length = %d",
                 esp_http_client_get_status_code(client),     //状态码
                 esp_http_client_get_content_length(client)); //数据长度
        ret = esp_http_client_read(client, pReadBuf, 512);    //读取512数据内容
        if (ret > 0)
        {
            ESP_LOGI(TAG, "recv data = %d %s", ret, pReadBuf); //打印数据
        }
        else
        {
            ESP_LOGI("esp_http_client_perform is failed\r\n", );
        }
    }

    esp_http_client_cleanup(client); //断开并释放资源
}

void http_client_task()
{
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .password = EXAMPLE_ESP_WIFI_PASS,
            /* Setting a password implies station will connect to all security modes including WEP/WPA.
             * However these modes are deprecated and not advisable to be used. Incase your Access point
             * doesn't support WPA2, these mode can be enabled by commenting below line */
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            .pmf_cfg = {
                .capable = true,
                .required = false},
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_LOGI(TAG, "wifi_init_sta finished.");
    while (1)
    {
        http_client_test();
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}

void cJSON_task()
{
    cJSON *root = NULL;
    cJSON *array = NULL;
    cJSON *item = NULL;
    cJSON *item_location = NULL;
    cJSON *item_now = NULL;
    E2213JS0C1_Init();
    while (1)
    {
        char buffer[512];
        BaseType_t ret;
        ret = xQueueReceive(QueueHandle_sys, buffer, portMAX_DELAY);
        if (ret != pdFAIL)
        {
            ESP_LOGI("cJSON_task", "buffer:%s", buffer);
            root = cJSON_Parse(buffer);
            if (!root)
            {
                printf("cJSON_Parse is error Error : [%s]\n", cJSON_GetErrorPtr());
            }
            else
            {
                printf("cJSON_Parse is OK\r\n");
                array = cJSON_GetObjectItem(root, "results");
                item = cJSON_GetArrayItem(array, 0);
                item_location = cJSON_GetObjectItem(item, "location");
                item_now = cJSON_GetObjectItem(item, "now");
                printf("---------------------------------------------------------\r\n");
                ESP_LOGI("sys_var", "temperature:%d", handler->temperature);
                if ((handler->temperature) != atoi((cJSON_GetObjectItem(item_now, "temperature")->valuestring)))
                {
                    char s[20];
                    handler->temperature = atoi((cJSON_GetObjectItem(item_now, "temperature")->valuestring));
                    E2213JS0C1_ShowCharStr(0, 0, "TEMP:", FONT_1608, BLACK, WHITE);
                    itoa(handler->temperature, s, 10); 
                    E2213JS0C1_ShowCharStr(5*8, 0, s, FONT_1608, BLACK, WHITE);
                    E2213JS0C1_SendImageData();
                    E2213JS0C1_SendUpdateCmd();
                    E2213JS0C1_TurnOffDCDC();
                    printf("change sys_var\r\n");
                }
                printf("timezone:%s\r\n", cJSON_GetObjectItem(item_location, "timezone")->valuestring);
                printf("name:%s\r\n", cJSON_GetObjectItem(item_location, "name")->valuestring);
                printf("id:%s\r\n", cJSON_GetObjectItem(item_location, "id")->valuestring);
                printf("text:%s\r\n", cJSON_GetObjectItem(item_now, "text")->valuestring);
                printf("temperature:%s\r\n", cJSON_GetObjectItem(item_now, "temperature")->valuestring);
                printf("---------------------------------------------------------\r\n");
            }
            cJSON_Delete(root);
        }
    }
}

void app_main(void)
{
    handler = (var)malloc(sizeof(variable));
    handler->temperature = 0;

    assert(handler != NULL);
    gpio_reset_pin(E2213JS0C1_CS);
    gpio_set_direction(E2213JS0C1_CS, GPIO_MODE_OUTPUT);
    gpio_reset_pin(E2213JS0C1_DC);
    gpio_set_direction(E2213JS0C1_DC, GPIO_MODE_OUTPUT);
    gpio_reset_pin(E2213JS0C1_RST);
    gpio_set_direction(E2213JS0C1_RST, GPIO_MODE_OUTPUT);
    gpio_reset_pin(E2213JS0C1_BUSY);
    gpio_set_direction(E2213JS0C1_BUSY, GPIO_MODE_INPUT);

    spi_bus_config_t buscfg = {
        .miso_io_num = 19,
        .mosi_io_num = PIN_NUM_MOSI,
        .sclk_io_num = 18,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 32,
    };

    spi_device_interface_config_t spi_interface = {
        .clock_speed_hz = SPI_MASTER_FREQ_20M,
        .mode = 0,
        .spics_io_num = -1,
        .queue_size = 1,

        .post_cb = spi_ready,
    };
    spi_bus_initialize(SPI3_HOST, &buscfg, SPI_DMA_CH_AUTO);
    spi_bus_add_device(SPI3_HOST, &spi_interface, &spi);
    QueueHandle_sys = xQueueCreate(10, 512);

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();
    s_wifi_event_group = xEventGroupCreate();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    xTaskCreate(Blink, "Blink", 8192, NULL, 5, NULL);
    xTaskCreate(&http_client_task, "http_client_task", 8192, NULL, 5, NULL);
    xTaskCreate(&cJSON_task, "cJSON_task", 8192, NULL, 5, NULL);

    // E2213JS0C1_ClearFullScreen(WHITE);
    // E2213JS0C1_SendImageData();
    // E2213JS0C1_SendUpdateCmd();
    // E2213JS0C1_TurnOffDCDC();
    // E2213JS0C1_ShowCharStr(0, 0, "Hello World", FONT_1608, BLACK, WHITE);
    // E2213JS0C1_SendImageData();
    // E2213JS0C1_SendUpdateCmd();
    // E2213JS0C1_TurnOffDCDC();
}
