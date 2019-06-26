#include "gfx.h"

#include <avr/pgmspace.h>
#include <avr/io.h>
#include <stdlib.h>

#define swap(a, b) { a = a ^ b; b = a ^ b; a = a ^ b; }

static int16_t display_width, display_height;
static int16_t cursor_x, cursor_y;

static uint16_t text_color;

static uint8_t text_size;
static uint8_t rotation;
static uint8_t text_wrap;

static void draw_fast_v_line(int16_t x, int16_t y,
			     int16_t height,
			     uint16_t color)
{
	LCD_set_range(x, y, x, y + height - 1);

	while (height--)
		LCD_set_pixel(color);
}

static void draw_circle(int16_t x, int16_t y,
			int16_t radius,
			uint16_t color)
{
	int16_t f = 1 - radius;
	int16_t ddF_x = 1, ddF_y = -2 * radius;
	int16_t aux_x = 0, aux_y = radius;

	LCD_draw_pixel(x, y + radius, color);
	LCD_draw_pixel(x, y - radius, color);
	LCD_draw_pixel(x + radius, y, color);
	LCD_draw_pixel(x - radius, y, color);

	while (aux_x < aux_y) {
		if (f >= 0) {
			aux_y--;
			ddF_y += 2;
			f += ddF_y;
		}

		aux_x++;

		ddF_x += 2;
		f += ddF_x;

		LCD_draw_pixel(x + aux_x, y + aux_y, color);
		LCD_draw_pixel(x - aux_x, y + aux_y, color);
		LCD_draw_pixel(x + aux_x, y - aux_y, color);
		LCD_draw_pixel(x - aux_x, y - aux_y, color);
		LCD_draw_pixel(x + aux_y, y + aux_x, color);
		LCD_draw_pixel(x - aux_y, y + aux_x, color);
		LCD_draw_pixel(x + aux_y, y - aux_x, color);
		LCD_draw_pixel(x - aux_y, y - aux_x, color);
	}
}

static void fill_circle(int16_t x, int16_t y,
			int16_t radius, uint8_t corner, int16_t delta,
			uint16_t color)
{

    int16_t f = 1 - radius;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * radius;
    int16_t aux_x = 0;
    int16_t aux_y = radius;

	while (aux_x < aux_y) {
		if (f >= 0) {
			aux_y--;
			ddF_y += 2;
			f += ddF_y;
		}
		
		aux_x++;

		ddF_x += 2;
		f += ddF_x;

		if (corner & 0x1) {
			draw_fast_v_line(x + aux_x, y - aux_y,
					 2 * aux_y + 1 + delta,
					 color);
			draw_fast_v_line(x + aux_y, y - aux_x,
					 2 * aux_x + 1 + delta,
					 color);
		}

		if (corner & 0x2) {
			draw_fast_v_line(x - aux_x, y - aux_y,
					 2 * aux_y + 1 + delta, color);
			draw_fast_v_line(x - aux_y, y - aux_x,
					 2 * aux_x + 1 + delta, color);
		}
	}
}

void GFX_init(int16_t width, int16_t height)
{
	display_width = width;
	display_height = height;

	cursor_y = cursor_x = 0;

	rotation = 0;

	text_size = 1;
	text_color = 0xFFFF;
	text_wrap = 1;
}

void GFX_draw_line(int16_t x1, int16_t y1,
		   int16_t x2, int16_t y2,
		   uint16_t color)
{
	int16_t dx, dy;
	int16_t err, ystep;
	int16_t gradient = abs(y2 - y1) > abs(x2 - x1);

	if (gradient) {
		swap(x1, y1);
		swap(x2, y2);
	}

	if (x1 > x2) {
		swap(x1, x2);
		swap(y1, y2);
	}

	dx = x2 - x1;
	dy = abs(y2 - y1);
	err = dx / 2;

	if (y1 < y2)
		ystep = 1;
	else
		ystep = -1;

	for (; x1 <= x2; x1++) {
		if (gradient)
			LCD_draw_pixel(y1, x1, color);
		else
			LCD_draw_pixel(x1, y1, color);
		
		err -= dy;
		
		if (err < 0) {
			y1 += ystep;
			err += dx;
		}
	}
}

void GFX_draw_circle(int16_t x, int16_t y,
		     int16_t radius,
		     uint16_t color, uint8_t fill)
{
	if (!fill) {
		draw_circle(x, y, radius, color);
	} else {
		draw_fast_v_line(x, y - radius, 2 * radius + 1, color);
		fill_circle(x, y, radius, 3, 0, color);
	}
}
