#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"

#include "sdkconfig.h"
#include "esp_log.h"
#include "E2213JS0C1.h"
#include "image.h"

#define EEPROM_HOST HSPI_HOST
#define PIN_NUM_MISO 18
#define PIN_NUM_MOSI 23
#define PIN_NUM_CLK 19

spi_device_handle_t spi;

static void IRAM_ATTR spi_ready(spi_transaction_t *trans)
{
}

void app_main(void)
{
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
    E2213JS0C1_Init();
    E2213JS0C1_ClearFullScreen(WHITE);
    E2213JS0C1_SendImageData();
    E2213JS0C1_SendUpdateCmd();
    vTaskDelay(100);
    E2213JS0C1_DrawImage(0, 0, 104, 212, gImage_1);
    E2213JS0C1_SendImageData();
    E2213JS0C1_SendUpdateCmd();
    E2213JS0C1_TurnOffDCDC();
    vTaskDelay(300);
    E2213JS0C1_ClearFullScreen(WHITE);
    E2213JS0C1_SendImageData();
    E2213JS0C1_SendUpdateCmd();
    E2213JS0C1_TurnOffDCDC();
    
    // E2213JS0C1_ClearFullScreen(WHITE);
    // E2213JS0C1_DrawPoint(0, 0, RED);
    // E2213JS0C1_DrawLine(0, 2, 10, HORIZONTAL, BLACK);
    // E2213JS0C1_DrawLine(0, 4, 10, VERTICAL, BLACK);
    // E2213JS0C1_DrawRectangle(0, 16, 10, 10, SOLID, BLACK, RED);
    // E2213JS0C1_DrawRectangle(20, 16, 10, 10, HOLLOW, BLACK, RED);
    // E2213JS0C1_ShowCharStr(0, 30, "FONT TEST", FONT_1608, BLACK, WHITE);
    // E2213JS0C1_DrawBmp(0, 50, 104, 41, BLACK, WHITE, BmpImage);
    // E2213JS0C1_ShowCharStr(0, 100, "UID:5572380", FONT_1608, BLACK, WHITE);
    // E2213JS0C1_ShowCharStr(20, 116, "Designed", FONT_1608, BLACK, WHITE);
    // E2213JS0C1_ShowCharStr(44, 132, "By", FONT_1608, BLACK, WHITE);
    // E2213JS0C1_ShowCharStr(40, 148, "szongen", FONT_1608, BLACK, WHITE);
    // E2213JS0C1_SendImageData();
    // E2213JS0C1_SendUpdateCmd();
    // E2213JS0C1_TurnOffDCDC();
}
