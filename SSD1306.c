#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include <stdint.h>
#include "i2cmaster.h"
#include "SSD1306.h"

#if 0
// save font in flash
#define FONT_MEM PROGMEM
#define FONT_READ_BYTE pgm_read_byte
#else
// save font in EEPROM
#define FONT_MEM EEMEM
#define FONT_READ_BYTE eeprom_read_byte
#endif

#define I2CADDR     0x78
#define WIRE_MAX    32

#define SSD1306_MEMORYMODE          0x20 ///< See datasheet
#define SSD1306_SEGREMAP            0xA0 ///< See datasheet
#define SSD1306_COMSCANDEC          0xC8 ///< See datasheet
#define SSD1306_SETMULTIPLEX        0xA8 ///< See datasheet
#define SSD1306_DISPLAYOFF          0xAE ///< See datasheet
#define SSD1306_SETDISPLAYCLOCKDIV  0xD5 ///< See datasheet
#define SSD1306_SETDISPLAYOFFSET    0xD3 ///< See datasheet
#define SSD1306_SETSTARTLINE        0x40 ///< See datasheet
#define SSD1306_CHARGEPUMP          0x8D ///< See datasheet
#define SSD1306_SETCOMPINS          0xDA ///< See datasheet
#define SSD1306_SETCONTRAST         0x81 ///< See datasheet
#define SSD1306_SETVCOMDETECT       0xDB ///< See datasheet
#define SSD1306_DISPLAYALLON_RESUME 0xA4 ///< See datasheet
#define SSD1306_NORMALDISPLAY       0xA6 ///< See datasheet
#define SSD1306_DEACTIVATE_SCROLL	0x2E ///< Stop scroll
#define SSD1306_DISPLAYON           0xAF ///< See datasheet
#define SSD1306_SETPRECHARGE        0xD9 ///< See datasheet
#define SSD1306_COLUMNADDR          0x21 ///< See datasheet
#define SSD1306_PAGEADDR            0x22 ///< See datasheet

static const uint8_t FONT_MEM _font[][7] = {
	{0x00,0x1C,0x3E,0x63,0x41,0x00,0x00},	// 0x28 (
	{0x00,0x41,0x63,0x3E,0x1C,0x00,0x00},	// 0x29 )
	{0x08,0x2A,0x3E,0x1C,0x1C,0x3E,0x2A},	// 0x2A *
	{0x08,0x08,0x3E,0x3E,0x08,0x08,0x00},	// 0x2B +
	{0x00,0xA0,0xE0,0x60,0x00,0x00,0x00},	// 0x2C ,
	{0x08,0x08,0x08,0x08,0x08,0x08,0x00},	// 0x2D -
	{0x00,0x00,0x60,0x60,0x00,0x00,0x00},	// 0x2E .
	{0x60,0x30,0x18,0x0C,0x06,0x03,0x01},	// 0x2F /
	{0x3E,0x7F,0x59,0x4D,0x7F,0x3E,0x00},	// 0x30 0
	{0x42,0x42,0x7F,0x7F,0x40,0x40,0x00},	// 0x31 1
	{0x62,0x73,0x59,0x49,0x6F,0x66,0x00},	// 0x32 2
	{0x22,0x63,0x49,0x49,0x7F,0x36,0x00},	// 0x33 3
	{0x18,0x1C,0x16,0x13,0x7F,0x7F,0x10},	// 0x34 4
	{0x27,0x67,0x45,0x45,0x7D,0x39,0x00},	// 0x35 5
	{0x3C,0x7E,0x4B,0x49,0x79,0x30,0x00},	// 0x36 6
	{0x03,0x63,0x71,0x19,0x0F,0x07,0x00},	// 0x37 7
	{0x36,0x7F,0x49,0x49,0x7F,0x36,0x00},	// 0x38 8
	{0x06,0x4F,0x49,0x69,0x3F,0x1E,0x00},	// 0x39 9
	{0x00,0x00,0x6C,0x6C,0x00,0x00,0x00},	// 0x3A :
	// {0x00,0xA0,0xEC,0x6C,0x00,0x00,0x00},	// 0x3B ;
	{0x46,0x66,0x30,0x18,0x0C,0x66,0x62},	// 0x25 %
	{0x08,0x1C,0x36,0x63,0x41,0x00,0x00},	// 0x3C <
	{0x14,0x14,0x14,0x14,0x14,0x14,0x00},	// 0x3D =
	{0x00,0x41,0x63,0x36,0x1C,0x08,0x00},	// 0x3E >
	{0x02,0x03,0x51,0x59,0x0F,0x06,0x00},	// 0x3F ?
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00},	// 0x20 ' '
	{0x7C,0x7E,0x13,0x13,0x7E,0x7C,0x00},	// 0x41 A
	{0x41,0x7F,0x7F,0x49,0x49,0x7F,0x36},	// 0x42 B
	{0x1C,0x3E,0x63,0x41,0x41,0x63,0x22},	// 0x43 C
	{0x41,0x7F,0x7F,0x41,0x63,0x7F,0x1C},	// 0x44 D
	{0x41,0x7F,0x7F,0x49,0x5D,0x41,0x63},	// 0x45 E
	{0x41,0x7F,0x7F,0x49,0x1D,0x01,0x03},	// 0x46 F
	{0x1C,0x3E,0x63,0x41,0x51,0x73,0x72},	// 0x47 G
	{0x7F,0x7F,0x08,0x08,0x7F,0x7F,0x00},	// 0x48 H
	{0x00,0x41,0x7F,0x7F,0x41,0x00,0x00},	// 0x49 I
	{0x30,0x70,0x40,0x41,0x7F,0x3F,0x01},	// 0x4A J
	{0x41,0x7F,0x7F,0x08,0x1C,0x77,0x63},	// 0x4B K
	{0x41,0x7F,0x7F,0x41,0x40,0x60,0x70},	// 0x4C L
	{0x7F,0x7F,0x06,0x0C,0x06,0x7F,0x7F},	// 0x4D M
	{0x7F,0x7F,0x06,0x0C,0x18,0x7F,0x7F},	// 0x4E N
	{0x1C,0x3E,0x63,0x41,0x63,0x3E,0x1C},	// 0x4F O
	{0x41,0x7F,0x7F,0x49,0x09,0x0F,0x06},	// 0x50 P
	{0x1E,0x3F,0x21,0x71,0x7F,0x5E,0x00},	// 0x51 Q
	{0x41,0x7F,0x7F,0x19,0x39,0x6F,0x46},	// 0x52 R
	{0x26,0x67,0x4D,0x59,0x7B,0x32,0x00},	// 0x53 S
	{0x03,0x41,0x7F,0x7F,0x41,0x03,0x00},	// 0x54 T
	{0x7F,0x7F,0x40,0x40,0x7F,0x7F,0x00},	// 0x55 U
	{0x1F,0x3F,0x60,0x60,0x3F,0x1F,0x00},	// 0x56 V
	{0x7F,0x7F,0x30,0x18,0x30,0x7F,0x7F},	// 0x57 W
	{0x63,0x77,0x1C,0x08,0x1C,0x77,0x63},	// 0x58 X
	{0x07,0x4F,0x78,0x78,0x4F,0x07,0x00},	// 0x59 Y
	{0x67,0x73,0x59,0x4D,0x47,0x63,0x71},	// 0x5A Z
	{0x00,0x00,0x10,0x28,0x10,0x00,0x00},	// 0x5B [ => animation
	{0x00,0x38,0x44,0x44,0x44,0x38,0x00},	// 0x5C '\' => animation
	{0x38,0x44,0x82,0x82,0x82,0x44,0x38},	// 0x5D ] => animation
	{0x00,0x06,0x09,0x09,0x06,0x00,0x00},	// 0x5E ^ => °
};

static void _SSD1306_command(const uint8_t c) {
	i2c_start_wait(I2CADDR+I2C_WRITE);
	i2c_write(0x00);	// Co = 0, D/C = 0
	i2c_write(c);
	i2c_stop();
}

static void _SSD1306_commandList(const uint8_t *c, uint8_t n, uint8_t fromFlash) {
	i2c_start_wait(I2CADDR+I2C_WRITE);
	i2c_write(0x00);
	uint8_t bytesOut = 1;
	while (n--) {
		if (bytesOut >= WIRE_MAX) {
			i2c_stop();
			i2c_start_wait(I2CADDR+I2C_WRITE);
			i2c_write(0x00);
			bytesOut = 1;
		}
		i2c_write(fromFlash ? pgm_read_byte(c) : *c);
		c++;
		bytesOut++;
	}
	i2c_stop();
}

void SSD1306_writeImg(uint8_t width, uint8_t height, const uint8_t *img, uint8_t fromFlash) {
	uint8_t x = 0;
	uint8_t y = 0;
	uint8_t ch;

	for (y=0; y < height/8; y++) {
		_SSD1306_command(SSD1306_COLUMNADDR);
		_SSD1306_command(0);
		_SSD1306_command(width-1);
		_SSD1306_command(SSD1306_PAGEADDR);
		_SSD1306_command(y);
		_SSD1306_command(y);

		i2c_start_wait(I2CADDR+I2C_WRITE);
		i2c_write(0x40);
		for (x=0; x < width; x++) {
			ch = fromFlash ? pgm_read_byte(img + (y * width) + x) : img[(y * width) + x];
			i2c_write(ch);
		}
		i2c_stop();
	}
}

void SSD1306_writeChar(uint8_t x, uint8_t y, uint8_t ch, uint8_t flags) {

	if (x > 15 || y > 7) return;	/* check if we're within our boundaries */
	if (ch == ' ') ch = '@';		/* map ' ' to '@' */
	else if (ch == '%') ch = ';';	/* map '%' to ';' */

	if (ch < '(' || ch > '^') ch = '@';
	ch -= '(';

    uint8_t loop;
    for (loop = 0; loop == 0 || (loop == 1 && (flags & SSD1306_FLAG_DOUBLE)); loop++) {
        /* it's actually much more compact to send these commands one by one than
         * building an array and sending them using commandList */
        _SSD1306_command(SSD1306_COLUMNADDR);
        _SSD1306_command(x * 8);
        _SSD1306_command(((x + (flags & SSD1306_FLAG_DOUBLE ? 2 : 1)) * 8) - 1);
        _SSD1306_command(SSD1306_PAGEADDR);
        _SSD1306_command(y + loop);
        _SSD1306_command(y + loop);

        i2c_start_wait(I2CADDR + I2C_WRITE);
        i2c_write(0x40);
        for (uint8_t line = 0; line < 7; ++line) {
            uint8_t c = (flags & SSD1306_FLAG_INVERTED) ? FONT_READ_BYTE(&_font[ch][line]) ^ 0xFF : FONT_READ_BYTE(
                    &_font[ch][line]);
            if (loop == 1) {
                c = ((c & 0x10) >> 4) | ((c & 0x10) >> 3) | ((c & 0x20) >> 3) | ((c & 0x20) >> 2) | ((c & 0x40) >> 2) |
                    ((c & 0x40) >> 1) | ((c & 0x80) >> 1) | (c & 0x80);
            } else if (flags & SSD1306_FLAG_DOUBLE) {
                c = (c & 0x01) | ((c & 0x01) << 1) | ((c & 0x02) << 1) | ((c & 0x02) << 2) | ((c & 0x04) << 2) |
                    ((c & 0x04) << 3) | ((c & 0x08) << 3) | ((c & 0x08) << 4);
            }
            i2c_write(c);
            if (flags & SSD1306_FLAG_DOUBLE) i2c_write(c);
        }
        i2c_write((flags & SSD1306_FLAG_INVERTED) ? 0xFF : 0x00);
        if (flags & SSD1306_FLAG_DOUBLE) i2c_write((flags & SSD1306_FLAG_INVERTED) ? 0xFF : 0x00);
        i2c_stop();
    }
}

uint8_t SSD1306_writeString(uint8_t x, uint8_t y, const char *str, uint8_t flags) {
	uint8_t ch;
	while ((ch = (flags & SSD1306_FLAG_PGM) ? pgm_read_byte(str++) : *str++) != '\0') {
		SSD1306_writeChar(x, y, ch, flags & ~(SSD1306_FLAG_PGM));
        x++;
        if (flags & SSD1306_FLAG_DOUBLE) x++;
	}
	return(x);
}

void SSD1306_clear(void) {
	/* clear display */
	_SSD1306_command(SSD1306_COLUMNADDR);
	_SSD1306_command(0x00);
	_SSD1306_command(0x7F);
	_SSD1306_command(SSD1306_PAGEADDR);
	_SSD1306_command(0x00);
	_SSD1306_command(0x07);
	for (uint8_t line = 64; line > 0; line--) {
		i2c_start_wait(I2CADDR+I2C_WRITE);
		i2c_write(0x40);
		for (uint8_t column = 0; column < 16; ++column) {
			i2c_write(0x00);
		}
		i2c_stop();
	}
}

void SSD1306_on(void) {
	_SSD1306_command(SSD1306_DISPLAYON);
}

void SSD1306_off(void) {
	_SSD1306_command(SSD1306_DISPLAYOFF);
}


void SSD1306_init(void) {
	static const uint8_t PROGMEM cmds[] = {
		SSD1306_DISPLAYOFF,
		SSD1306_SETDISPLAYCLOCKDIV,
		0x80,
		SSD1306_SETMULTIPLEX,
		0x3F,
		SSD1306_SETDISPLAYOFFSET,
		0x00,
		SSD1306_SETSTARTLINE | 0x00,
		SSD1306_CHARGEPUMP,
		0x14,
		SSD1306_MEMORYMODE,
		0x00,
		SSD1306_SEGREMAP | 0x01,
		SSD1306_COMSCANDEC,
		SSD1306_SETCOMPINS,
		0x12,
		SSD1306_SETCONTRAST,
		0xCF,
		SSD1306_SETPRECHARGE,
		0xF1,
		SSD1306_SETVCOMDETECT,
		0x40,
		SSD1306_DISPLAYALLON_RESUME,
		SSD1306_NORMALDISPLAY
	};

	_SSD1306_commandList(cmds, sizeof(cmds), 1);
}

/**
 * C++ version 0.4 char* style "itoa":
 * Written by Lukás Chmela
 * Released under GPLv3.
 */
uint8_t SSD1306_writeInt(uint8_t x, uint8_t y, int16_t value, uint8_t base, uint8_t flags, uint8_t len) {
	static const char PROGMEM data[] = "ZYXWVUTSRQPONMLKJIHGFEDCBA9876543210123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	char result[17];
	if (base < 2 || base > 36 || len > 16) return(x);

	char *ptr = result, *ptr1 = result, tmp_char;
	int16_t tmp_value;

	do {
		tmp_value = value;
		value /= base;
		*ptr++ = pgm_read_byte(data + (35 + (tmp_value - value * base)));
	} while (value);

	if (tmp_value < 0) *ptr++ = '-';

    while (ptr - result < len) *ptr++ = (flags & SSD1306_FLAG_FILL_ZERO) ? '0' : ' ';

    *ptr-- = '\0';
	while (ptr1 < ptr) {
		tmp_char = *ptr;
		*ptr--= *ptr1;
		*ptr1++ = tmp_char;
	}

	return(SSD1306_writeString(x, y, result, flags & ~(SSD1306_FLAG_PGM)));
}
