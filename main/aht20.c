#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_log.h"
#include "driver/i2c.h"
#include "aht20.h"

static const char *AHT20 = "AHT20";

// Initialize I2C master peripheral
esp_err_t i2c_master_init(void) {
	i2c_config_t config = {
		.mode = I2C_MODE_MASTER,
		.sda_io_num = I2C_SDA_IO,
		.sda_pullup_en = GPIO_PULLUP_DISABLE,
		.scl_io_num = I2C_SCL_IO,
		.scl_pullup_en = GPIO_PULLUP_DISABLE,
		.clk_stretch_tick = 500, // Extend clock stretching timeout
	};

	esp_err_t err = i2c_driver_install(I2C_NUM_0, config.mode);
	if (err != ESP_OK) {
		return err;
	}

	return i2c_param_config(I2C_NUM_0, &config);
}

// Read status byte from AHT20
esp_err_t aht20_read_status(uint8_t *status) {
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (AHT20_ADDR << 1) | I2C_MASTER_READ, ACK_CHECK_EN);
	i2c_master_read_byte(cmd, status, I2C_MASTER_NACK);
	i2c_master_stop(cmd);
	esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS);
	i2c_cmd_link_delete(cmd);

	return ret;
}

// Initialize AHT20 sensor calibration
esp_err_t aht20_init_sensor(void) {
	uint8_t status = 0;

	vTaskDelay(40 / portTICK_RATE_MS); // Wait 40ms after power-on

	if (aht20_read_status(&status) != ESP_OK) {
		ESP_LOGE(AHT20, "Failed to connect to AHT20");
		return ESP_FAIL;
	}

	// Check if calibrated bit (bit 3) is set
	if ((status & 0x08) == 0) {
		// Send initialization command if not calibrated
		i2c_cmd_handle_t cmd = i2c_cmd_link_create();
		i2c_master_start(cmd);
		i2c_master_write_byte(cmd, (AHT20_ADDR << 1) | I2C_MASTER_WRITE, ACK_CHECK_EN);
		i2c_master_write_byte(cmd, 0xBE, ACK_CHECK_EN); // Initialization
		i2c_master_write_byte(cmd, 0x08, ACK_CHECK_EN);
		i2c_master_write_byte(cmd, 0x00, ACK_CHECK_EN);
		i2c_master_stop(cmd);
		i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS);
		i2c_cmd_link_delete(cmd);

		vTaskDelay(10 / portTICK_RATE_MS);
	}

	return ESP_OK;
}

// Read raw metrics and convert to floating numbers
void aht20_task(void *pvParameters) {
	uint8_t data[6];

	if (aht20_init_sensor() != ESP_OK) {
		vTaskDelete(NULL);
		return;
	}

	while (1) {
			// Trigger measurement
			i2c_cmd_handle_t cmd = i2c_cmd_link_create();
			i2c_master_start(cmd);
			i2c_master_write_byte(cmd, (AHT20_ADDR << 1) | I2C_MASTER_WRITE, ACK_CHECK_EN);
			i2c_master_write_byte(cmd, 0xAC, ACK_CHECK_EN); // Trigger command
			i2c_master_write_byte(cmd, 0x33, ACK_CHECK_EN);
			i2c_master_write_byte(cmd, 0x00, ACK_CHECK_EN);
			i2c_master_stop(cmd);
			i2c_master_cmd_begin(I2C_NUM_0, cmd, (1000 / portTICK_RATE_MS));
			i2c_cmd_link_delete(cmd);

			// Wait for measurement to complete (at least 80ms)
			vTaskDelay(85 / portTICK_RATE_MS);

			// Read 6 bytes of data
			cmd = i2c_cmd_link_create();
			i2c_master_start(cmd);
			i2c_master_write_byte(cmd, (AHT20_ADDR << 1) | I2C_MASTER_READ, ACK_CHECK_EN);
			for (int i = 0; i < 5; i++) {
				i2c_master_read_byte(cmd, &data[i], ACK_VAL);
			}
			i2c_master_read_byte(cmd, &data[5], NACK_VAL);
			i2c_master_stop(cmd);

			esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, (1000 / portTICK_RATE_MS));
			i2c_cmd_link_delete(cmd);

			if (ret == ESP_OK) {
				// Check if sensor is done busy (bit 7 should be 0)
				if ((data[0] & 0x80) == 0) {
					// Parse 20-bit raw data strings
					uint32_t raw_temperature = ((((uint32_t)data[3]) & 0x0F) << 16) | (((uint32_t)data[4]) << 8) | ((uint32_t)data[5]);
					uint32_t raw_humidity = (((uint32_t)data[1]) << 12) | (((uint32_t)data[2]) << 4) | (((uint32_t)data[3]) >> 4);

					float temperature = (((float)raw_temperature / 1048576.0) * 200.0) - 50.0;
					float humidity = ((float)raw_humidity / 1048576.0) * 100.0;

					uint32_t t = temperature * 100;
					uint32_t h = humidity * 100;

					ESP_LOGI(AHT20, "Temperature: %d.%d °C, Humidity: %d.%d %%", t / 100, t % 100, h / 100, h % 100);
				} else {
					ESP_LOGW(AHT20, "Sensor busy...");
				}
			} else {
				ESP_LOGE(AHT20, "Failed to read data over I2C");
			}

			vTaskDelay(2000 / portTICK_RATE_MS); // Sample every 2 seconds
	}
}
