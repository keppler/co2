/*         ___    ___
 *  __ ___|_  )__/ __| ___ _ _  ___ ___ _ _
 * / _/ _ \/ /___\__ \/ -_) ' \(_-</ _ \ '_|
 *_\__\___/___|  |___/\___|_||_/__/\___/_|_________________________________
 * CO₂ Sensor for Caving -- https://github.com/keppler/co2
 * SSD1306 display driver
 */

#ifndef _SSD1306_H
#define _SSD1306_H

#define SSD1306_FLAG_PGM       0x01
#define SSD1306_FLAG_INVERTED  0x02
#define SSD1306_FLAG_DOUBLE    0x04
#define SSD1306_FLAG_FILL_ZERO 0x08
#define SSD1306_FLAG_LIGHT     0x10

void SSD1306_init(void);
void SSD1306_writeImg(uint8_t x, uint8_t y, uint8_t width, uint8_t height, const uint8_t *img, uint8_t src); /* src: 0=mem, 1=pgm, 2=eeprom */
void SSD1306_writeChar(uint8_t x, uint8_t y, uint8_t ch, uint8_t flags);
uint8_t SSD1306_writeString(uint8_t x, uint8_t y, const char *str, uint8_t flags);
void SSD1306_clear(void);
void SSD1306_on(void);
void SSD1306_off(void);
uint8_t SSD1306_writeInt(uint8_t x, uint8_t y, int32_t value, uint8_t base, uint8_t flags, uint8_t len);

#endif /* !_SSD1306_H */
