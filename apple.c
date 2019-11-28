/*********************************************************/
/* Copyright - Jon Gutschon														   */
/*********************************************************/

#include "GLCD.h"
#include "graphics.h"

#define APPLE_WIDTH (20)
#define APPLE_HEIGHT (20)
#define APPLE_BYTES_PER_PIXEL (2) /* 2:RGB16, 3:RGB, 4:RGBA */
#define APPLE_PIXEL_DATA ((unsigned char*) APPLE_pixel_data)

static const unsigned char APPLE_pixel_data[20 * 20 * 2 + 1] =
("\000\000\000\000\000\000\000\000\000\000\000\000\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000"
 "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\370\000\370\000\370\000\370\000\370\000\370"
 "\000\370\000\370\000\370\000\370\000\370\000\370\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\370\000\370"
 "\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000"
 "\000\000\000\000\000\000\000\000\000\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000"
 "\370\000\370\000\370\000\370\000\370\000\370\000\370\000\000\000\000\000\000\000\370\000\370\000\370\000\370"
 "\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000"
 "\370\000\370\000\000\000\000\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000"
 "\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\000\000\370\000\370\000\370"
 "\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000"
 "\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370"
 "\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000"
 "\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370"
 "\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000"
 "\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370"
 "\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000"
 "\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370"
 "\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000"
 "\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370"
 "\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000"
 "\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370"
 "\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\000\000\370\000\370\000\370\000\370\000\370"
 "\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000"
 "\370\000\000\000\000\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000"
 "\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\000\000\000\000\000\000\370\000\370\000\370"
 "\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000"
 "\370\000\000\000\000\000\000\000\000\000\000\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000"
 "\370\000\370\000\370\000\370\000\370\000\370\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\370\000\370"
 "\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\000\000\000\000\000\000"
 "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\370\000\370\000\370\000\370\000\370\000\370\000\370\000\370"
 "\000\000\000\000\000\000\000\000\000\000\000\000");

void drawApple(int x, int y) {
	GLCD_Bitmap(x, y, 20, 20, APPLE_PIXEL_DATA);
}
