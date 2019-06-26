#define __PROG_TYPES_COMPAT__

#include "lcd.h"

#include <avr/io.h>
#include <avr/interrupt.h>

#include <limits.h>
#include <util/delay.h>

static void SPI_init(void)
{
	SPI_REG |= (1 << LCD_CMD);
	SPI_REG |= (1 << LCD_RST);
	SPI_REG |= (1 << LCD_CS);
	SPI_REG |= (1 << LCD_SCK);
	SPI_REG |= (1 << LCD_MOSI);
	SPI_REG |= (1 << PB4);

	SPI_PORT &= ~(1 << LCD_CS);
	SPI_PORT &= ~(1 << LCD_SCK);
	SPI_PORT &= ~(1 << LCD_MOSI);
	SPI_PORT |= (1 << PB4);

	SPCR |= (1 << MSTR) | (1 << SPE);
}

static void SPI_end(void)
{
	SPCR &= ~(1 << SPE);
}

static void SPI_set_bit_order(uint8_t bit_order)
{
	if (bit_order == LSB_FIRST)
		SPCR |= (1 << DORD);
	else
		SPCR &= ~(1 << DORD);
}

static void SPI_set_mode(uint8_t mode)
{
	SPCR = (SPCR & ~SPI_MODE_MASK) | mode;
}

static void SPI_set_clock_divider(uint8_t rate)
{
	SPCR = (SPCR & ~SPI_CLOCK_MASK) | (rate & SPI_CLOCK_MASK);
	SPSR = (SPSR & ~SPI_2XCLOCK_MASK) | ((rate >> 2) & SPI_2XCLOCK_MASK);
}

static inline void SPI_write(uint8_t byte)
{
	SPDR = byte;

	while (!(SPSR & (1 << SPIF)))
		;
}

static void SPI_write_cmd(uint8_t cmd)
{
	SPI_PORT &= ~(1 << LCD_CMD);
	SPI_write(cmd);
}

static void SPI_write_data(uint8_t data)
{
	SPI_PORT |= (1 << LCD_CMD);
	SPI_write(data);
}

PROGMEM static const prog_uchar
ST7735B_CMD[] = {
	18,
	ST7735_SWRESET, ST7735_DELAY,
	50,
	ST7735_SLPOUT, ST7735_DELAY,
	255,
	ST7735_COLMOD, ST7735_DELAY + 1,
	0x05, 10,
	ST7735_FRMCTR1, ST7735_DELAY + 3,
	0x00, 0x06, 0x03, 10,
	ST7735_MADCTL, 1,
	0x08,
	ST7735_DISSET5, 2,
	0x15, 0x02,
	ST7735_INVCTR, 1,
	0x0,
	ST7735_PWCTR1, ST7735_DELAY + 2,
	0x02, 0x70, 10,
	ST7735_PWCTR2, 1,
	0x05,
	ST7735_PWCTR3, 2,
	0x01, 0x02,
	ST7735_VMCTR1, ST7735_DELAY + 2,
	0x3C, 0x38, 10,
	ST7735_PWCTR6, 2,
	0x11, 0x15,
	ST7735_GMCTRP1, 16,
	0x09, 0x16, 0x09, 0x20,
	0x21, 0x1B, 0x13, 0x19,
	0x17, 0x15, 0x1E, 0x2B,
	0x04, 0x05, 0x02, 0x0E,
	ST7735_GMCTRN1, ST7735_DELAY + 16,
	0x0B, 0x14, 0x08, 0x1E,
	0x22, 0x1D, 0x18, 0x1E,
	0x1B, 0x1A, 0x24, 0x2B,
	0x06, 0x06, 0x02, 0x0F,
	10,
	ST7735_CASET, 4,
	0x00, 0x02, 0x00, 0x81,
	ST7735_RASET, 4,
	0x00, 0x02, 0x00, 0x81,
	ST7735_NORON, ST7735_DELAY,
	10,
	ST7735_DISPON, ST7735_DELAY,
	255
};

PROGMEM static const prog_uchar
ST7735R_CMD1[] = {
	15,
	ST7735_SWRESET, ST7735_DELAY,
	150,
	ST7735_SLPOUT, ST7735_DELAY,
	255,
	ST7735_FRMCTR1, 3,
	0x01, 0x2C, 0x2D,
	ST7735_FRMCTR2, 3,
	0x01, 0x2C, 0x2D,
	ST7735_FRMCTR3, 6,
	0x01, 0x2C, 0x2D,
	0x01, 0x2C, 0x2D,
	ST7735_INVCTR, 1,
	0x07,
	ST7735_PWCTR1 , 3,
	0xA2, 0x02, 0x84,
	ST7735_PWCTR2 , 1,
	0xC5,
	ST7735_PWCTR3 , 2,
	0x0A, 0x00,
	ST7735_PWCTR4 , 2,
	0x8A, 0x2A,
	ST7735_PWCTR5 , 2,
	0x8A, 0xEE,
	ST7735_VMCTR1 , 1,
	0x0E,
	ST7735_INVOFF , 0,
	ST7735_MADCTL , 1,
	0xC8,
	ST7735_COLMOD , 1,
	0x05
};

PROGMEM static const prog_uchar
ST7735R_CMD2[] = {
	2,
	ST7735_CASET , 4,
	0x00, 0x02, 0x00, 0x7F + 0x02,
	ST7735_RASET , 4,
	0x00, 0x01, 0x00, 0x9F + 0x01
};

PROGMEM static const prog_uchar
ST7735R_CMD3[] = {
	2,
	ST7735_CASET, 4,
	0x00, 0x00, 0x00, 0x7F,
	ST7735_RASET, 4,
	0x00, 0x00, 0x00, 0x9F
};

PROGMEM static const prog_uchar
ST7735R_CMD4[] = {
	4,
	ST7735_GMCTRP1, 16,
	0x02, 0x1c, 0x07, 0x12,
	0x37, 0x32, 0x29, 0x2d,
	0x29, 0x25, 0x2B, 0x39,
	0x00, 0x01, 0x03, 0x10,
	ST7735_GMCTRN1, 16,
	0x03, 0x1d, 0x07, 0x06,
	0x2E, 0x2C, 0x29, 0x2D,
	0x2E, 0x2E, 0x37, 0x3F,
	0x00, 0x00, 0x02, 0x10,
	ST7735_NORON, ST7735_DELAY,
	10,
	ST7735_DISPON, ST7735_DELAY,
	100
};

static void parse_cmd(const uint8_t *addr)
{
	uint8_t num_cmd, num_args;
	uint16_t ms;

	num_cmd = pgm_read_byte(addr++);
	while (num_cmd--) {
		SPI_write_cmd(pgm_read_byte(addr++));
		num_args = pgm_read_byte(addr++);

		ms = num_args & ST7735_DELAY;
		num_args &= ~ST7735_DELAY;

		while (num_args--)
			SPI_write_data(pgm_read_byte(addr++));

		if (ms) {
			ms = pgm_read_byte(addr++);

			if(ms == 255)
				ms = 500;

			_delay_ms(ms);
		}
	}
}

static void set_backlight(void)
{
	SPI_REG |= (1 << LCD_BL);

	TCCR0A |= (1 << COM0A1) | (1 << WGM00);
	TCCR0B |= (1 << CS00);

	OCR0A = ST7735_BRIGHTNESS;
}

static int display_width = 128;
static int display_height = 128;
static uint8_t col, row;

void LCD_init(void)
{
	col = row = 0;	

	SPI_init();
	
	SPI_set_clock_divider(SPI_CLOCK_DIV4);
	SPI_set_bit_order(MSB_FIRST);
	SPI_set_mode(SPI_MODE0);

	SPI_PORT |= (1 << LCD_RST);
	_delay_ms(100);
	SPI_PORT &= ~(1 << LCD_RST);
	_delay_ms(100);
	SPI_PORT |= (1 << LCD_RST);
	_delay_ms(100);
	
	parse_cmd(ST7735R_CMD1);
	parse_cmd(ST7735R_CMD3);
	parse_cmd(ST7735R_CMD4);

	set_backlight();	
}

void LCD_end(void)
{
	SPI_end();
}

void LCD_set_range(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
	SPI_write_cmd(ST7735_CASET);

	SPI_write_data(0x00);
	SPI_write_data(x1 + col);
	SPI_write_data(0x00);
	SPI_write_data(x2 + col);

	SPI_write_cmd(ST7735_RASET);

	SPI_write_data(0x00);
	SPI_write_data(y1 + row);
	SPI_write_data(0x00);
	SPI_write_data(y2 + row);

	SPI_write_cmd(ST7735_RAMWR);
}

void LCD_set_pixel(uint16_t color)
{
	SPI_PORT |= (1 << LCD_CMD);
	
	SPI_write(color >> 8);
        SPI_write(color);
}

void LCD_draw_pixel(int16_t x, int16_t y, uint16_t color)
{
	if ((x < 0) || (x >= display_width) ||
	    (y < 0) || (y >= display_height))
		return;

	LCD_set_range(x, y, x + 1, y + 1);
	LCD_set_pixel(color);
}

void LCD_fill_screen(uint16_t color)
{
	uint8_t x, y;

	LCD_set_range(0, 0, display_width - 1, display_height - 1);

	for (y = display_height; y > 0; y--)
		for (x = display_width; x > 0; x--)
			LCD_set_pixel(color);
}
