#ifndef __GFX_H_
#define __GFX_H_

#include <avr/io.h>
#include "lcd.h"

void GFX_init(int16_t width, int16_t height);

void GFX_draw_line(int16_t x1, int16_t y1,
		   int16_t x2, int16_t y2,
		   uint16_t color);

void GFX_draw_circle(int16_t x, int16_t y,
		     int16_t radius,
		     uint16_t color, uint8_t fill);

#endif /* GFX_H_ */
