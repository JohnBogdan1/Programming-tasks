#ifndef __LCD_H_
#define __LCD_H_

#include <avr/pgmspace.h>

#define SPI_REG			DDRB
#define SPI_PORT		PORTB

#define LCD_CS			PB0
#define LCD_RST			PB1
#define LCD_CMD			PB2
#define LCD_BL			PB3
#define LCD_NC			PB4
#define LCD_MOSI		PB5
#define LCD_SCK			PB7

void LCD_init(void);
void LCD_end(void);

void LCD_set_range(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
void LCD_set_pixel(uint16_t color);

void LCD_draw_pixel(int16_t x, int16_t y, uint16_t color);
void LCD_fill_screen(uint16_t color);

#define LSB_FIRST			0
#define MSB_FIRST			1

#define SPI_CLOCK_DIV4		0x00
#define SPI_CLOCK_DIV16		0x01
#define SPI_CLOCK_DIV64		0x02
#define SPI_CLOCK_DIV128	0x03
#define SPI_CLOCK_DIV2		0x04
#define SPI_CLOCK_DIV8		0x05
#define SPI_CLOCK_DIV32		0x06

#define SPI_MODE0			0x00
#define SPI_MODE1			0x04
#define SPI_MODE2			0x08
#define SPI_MODE3			0x0C

#define SPI_MODE_MASK		0x0C
#define SPI_CLOCK_MASK		0x03
#define SPI_2XCLOCK_MASK	0x01

#define INITR_GREEN_TAB		0x00
#define INITR_RED_TAB		0x01

#define ST7735_TFT_WIDTH	128
#define ST7735_TFT_HEIGHT	160

#define ST7735_NOP			0x00
#define ST7735_SWRESET		0x01
#define ST7735_RDDID		0x04
#define ST7735_RDDST		0x09

#define ST7735_SLPIN		0x10
#define ST7735_SLPOUT		0x11
#define ST7735_PTLON		0x12
#define ST7735_NORON		0x13

#define ST7735_INVOFF		0x20
#define ST7735_INVON		0x21
#define ST7735_DISPOFF		0x28
#define ST7735_DISPON		0x29
#define ST7735_CASET		0x2A
#define ST7735_RASET		0x2B
#define ST7735_RAMWR		0x2C
#define ST7735_RAMRD		0x2E

#define ST7735_PTLAR		0x30
#define ST7735_COLMOD		0x3A
#define ST7735_MADCTL		0x36

#define ST7735_FRMCTR1		0xB1
#define ST7735_FRMCTR2		0xB2
#define ST7735_FRMCTR3		0xB3
#define ST7735_INVCTR		0xB4
#define ST7735_DISSET5		0xB6

#define ST7735_PWCTR1		0xC0
#define ST7735_PWCTR2		0xC1
#define ST7735_PWCTR3		0xC2
#define ST7735_PWCTR4		0xC3
#define ST7735_PWCTR5		0xC4
#define ST7735_VMCTR1		0xC5

#define ST7735_RDID1		0xDA
#define ST7735_RDID2		0xDB
#define ST7735_RDID3		0xDC
#define ST7735_RDID4		0xDD

#define ST7735_PWCTR6		0xFC

#define ST7735_DELAY		0x80

#define ST7735_GMCTRP1		0xE0
#define ST7735_GMCTRN1		0xE1

#define ST7735_BRIGHTNESS	0x006
#define ST7735_BLACK		0x0000
#define ST7735_RED			0xF800
#define ST7735_BLUE			0x001F
#define ST7735_GREEN		0x07E0
#define ST7735_YELLOW		0xFFE0
#define ST7735_MAGENTA		0xF81F
#define ST7735_CYAN			0x07FF
#define ST7735_WHITE		0xFFFF

#endif	/* LCD_H_ */
