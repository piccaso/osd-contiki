#ifndef __SKETCH_h__
#define __SKETCH_h__

#ifdef __cplusplus
extern "C" {
#endif

size_t color_to_string (const char *name, const char *uri, char *buf, size_t bsize);
int color_from_string (const char *name, const char *uri, const char *s);
int color_rgb_from_string (const char *r, const char *g, const char *b);
int leds_set_color_rgb(byte led, byte red, byte green, byte blue);

#ifdef __cplusplus
}
#endif

#endif
