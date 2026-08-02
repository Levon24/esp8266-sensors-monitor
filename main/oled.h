#ifndef OLED_H
#define OLED_H

#ifdef __cplusplus
extern "C" {
#endif

//
#define OLED_DC_GPIO    12
#define OLED_RST_GPIO   15
#define OLED_PIN_SEL		(1ULL << OLED_DC_GPIO) | (1ULL << OLED_RST_GPIO)

//
esp_err_t oled_delay_ms(uint32_t time);
esp_err_t oled_set_dc(uint8_t dc);
esp_err_t oled_write_cmd(uint8_t data);
esp_err_t oled_rst();
esp_err_t oled_init();
esp_err_t oled_set_pos(uint8_t x_start, uint8_t y_start);
esp_err_t oled_clear(uint8_t data);
void oled_task(void *pvParameters);

#ifdef __cplusplus
}
#endif

#endif // OLED_H
