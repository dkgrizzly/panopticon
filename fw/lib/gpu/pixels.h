#ifndef _PIXELS_H_
#define _PIXELS_H_

#define PIXELS_X    320
#define PIXELS_Y    240

void PutPixel(uint16_t x, uint16_t y, uint8_t c);
void DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t c);
void DrawHLine(uint16_t x1, uint16_t x2, uint16_t y, uint8_t c);
void DrawVLine(uint16_t x, uint16_t y1, uint16_t y2, uint8_t c);
void DrawRect(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t c);
void FillRect(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t c);

#endif // _PIXELS_H_

