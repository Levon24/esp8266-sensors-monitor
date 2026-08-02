#ifndef AHT20_H
#define AHT20_H

#ifdef __cplusplus
extern "C" {
#endif

//
#define I2C_SCL_IO		5				// GPIO pin for I2C clock
#define I2C_SDA_IO		4				// GPIO pin for I2C data
#define I2C_FREQ_HZ		100000	// I2C master clock frequency (100kHz)

#define AHT20_ADDR		0x38		// AHT20 I2C address

#define ACK_CHECK_EN  0x01		// I2C master will check ack from slave
#define ACK_VAL       0x00		// I2C ack value
#define NACK_VAL      0x01		// I2C nack value

//
esp_err_t i2c_master_init(void);
esp_err_t aht20_read_status(uint8_t *status);
esp_err_t aht20_init_sensor(void);
void aht20_task(void *pvParameters);

#ifdef __cplusplus
}
#endif

#endif // AHT20_H
