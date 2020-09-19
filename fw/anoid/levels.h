// Bricks are layed out on a 16x16 pixel grid, play field is 416 pixels wide, by TBD pixels tall.

#define BRICK_WHITE   5
#define BRICK_ORANGE  6
#define BRICK_CYAN    7
#define BRICK_GREEN   8
#define BRICK_RED     9
#define BRICK_BLUE    10
#define BRICK_PINK    11
#define BRICK_YELLOW  12
#define BRICK_SILVER  13
#define BRICK_GOLD    14

typedef struct brick_s {
  uint8_t x, y;
  uint8_t t;
} brick_t;

brick_t stage_1[] = {
  { 0, 4, BRICK_SILVER },  { 2, 4, BRICK_SILVER },  { 4, 4, BRICK_SILVER },  { 6, 4, BRICK_SILVER },
  { 8, 4, BRICK_SILVER },  { 10, 4, BRICK_SILVER },  { 12, 4, BRICK_SILVER },  { 14, 4, BRICK_SILVER },
  { 16, 4, BRICK_SILVER },  { 18, 4, BRICK_SILVER },  { 20, 4, BRICK_SILVER },  { 22, 4, BRICK_SILVER },
  { 24, 4, BRICK_SILVER },

  { 0, 5, BRICK_RED },  { 2, 5, BRICK_RED },  { 4, 5, BRICK_RED },  { 6, 5, BRICK_RED },  { 8, 5, BRICK_RED },
  { 10, 5, BRICK_RED },  { 12, 5, BRICK_RED },  { 14, 5, BRICK_RED },  { 16, 5, BRICK_RED },  { 18, 5, BRICK_RED },
  { 20, 5, BRICK_RED },  { 22, 5, BRICK_RED },  { 24, 5, BRICK_RED },

  { 0, 6, BRICK_YELLOW },  { 2, 6, BRICK_YELLOW },  { 4, 6, BRICK_YELLOW },  { 6, 6, BRICK_YELLOW },
  { 8, 6, BRICK_YELLOW },  { 10, 6, BRICK_YELLOW },  { 12, 6, BRICK_YELLOW },  { 14, 6, BRICK_YELLOW },
  { 16, 6, BRICK_YELLOW },  { 18, 6, BRICK_YELLOW },  { 20, 6, BRICK_YELLOW },  { 22, 6, BRICK_YELLOW },
  { 24, 6, BRICK_YELLOW },

  { 0, 7, BRICK_BLUE },  { 2, 7, BRICK_BLUE },  { 4, 7, BRICK_BLUE },  { 6, 7, BRICK_BLUE },  { 8, 7, BRICK_BLUE },
  { 10, 7, BRICK_BLUE },  { 12, 7, BRICK_BLUE },  { 14, 7, BRICK_BLUE },  { 16, 7, BRICK_BLUE },  { 18, 7, BRICK_BLUE },
  { 20, 7, BRICK_BLUE },  { 22, 7, BRICK_BLUE },  { 24, 7, BRICK_BLUE },

  { 0, 8, BRICK_PINK },  { 2, 8, BRICK_PINK },  { 4, 8, BRICK_PINK },  { 6, 8, BRICK_PINK },  { 8, 8, BRICK_PINK },
  { 10, 8, BRICK_PINK },  { 12, 8, BRICK_PINK },  { 14, 8, BRICK_PINK },  { 16, 8, BRICK_PINK },  { 18, 8, BRICK_PINK },
  { 20, 8, BRICK_PINK },  { 22, 8, BRICK_PINK },  { 24, 8, BRICK_PINK },

  { 0, 8, BRICK_GREEN },  { 2, 8, BRICK_GREEN },  { 4, 8, BRICK_GREEN },  { 6, 8, BRICK_GREEN },  { 8, 8, BRICK_GREEN },
  { 10, 8, BRICK_GREEN },  { 12, 8, BRICK_GREEN },  { 14, 8, BRICK_GREEN },  { 16, 8, BRICK_GREEN },
  { 18, 8, BRICK_GREEN },  { 20, 8, BRICK_GREEN },  { 22, 8, BRICK_GREEN },  { 24, 8, BRICK_GREEN },
};
