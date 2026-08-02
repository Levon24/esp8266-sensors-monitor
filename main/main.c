#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "driver/i2c.h"
#include "esp_log.h"
#include "main.h"
#include "wifi.h"
#include "oled.h"
#include "aht20.h"

/**
 * Main
 */
void app_main(void) {
	printf("Initialization!\n");

	/* Print chip information */
	esp_chip_info_t chip_info;
	esp_chip_info(&chip_info);
	printf("ESP8266 chip with %d CPU cores.\n", chip_info.cores);
	printf("Silicon revision %d.\n", chip_info.revision);
	printf("%dMB %s flash\n", spi_flash_get_chip_size() / (1024 * 1024),
			(chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

	// WiFi
  ESP_ERROR_CHECK(nvs_flash_init());
  wifi_init_station();

  // OLED
  xTaskCreate(oled_task, "oled", 2048, NULL, 5, NULL);

	// Sensors
  ESP_ERROR_CHECK(i2c_master_init());
  xTaskCreate(aht20_task, "aht20", 2048, NULL, 5, NULL);
}
