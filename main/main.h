#ifdef __cplusplus
extern "C" {
#endif

#define I2C_SCL_IO		5				// GPIO pin for I2C clock
#define I2C_SDA_IO		4				// GPIO pin for I2C data
#define I2C_FREQ_HZ		100000	// I2C master clock frequency (100kHz)

#define AHT20_ADDR		0x38		// AHT20 I2C address

#define ACK_CHECK_EN  0x1			// I2C master will check ack from slave
#define ACK_VAL       0x0			// I2C ack value
#define NACK_VAL      0x1			// I2C nack value

#define PRECISION			10000

#ifdef __cplusplus
}
#endif
