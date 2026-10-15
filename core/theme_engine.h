#ifndef THEME_ENGINE_H
#define THEME_ENGINE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Colores en formato PSP 32-bit little-endian (0xAABBGGRR) */
#define COLOR_RGBA(r, g, b, a) ((uint32_t)(((a) << 24) | ((b) << 16) | ((g) << 8) | (r)))
#define COLOR_RGB(r, g, b)     COLOR_RGBA(r, g, b, 255)

typedef enum {
    THEME_DARK = 0,     /* Cyberpunk / Minimal Dark */
    THEME_SEPIA = 1,    /* Sepia Paper para lectura prolongada */
    THEME_OLED = 2,     /* Alto contraste blanco sobre negro */
    THEME_COUNT
} ThemeType;

typedef struct {
    const char *name;
    uint32_t bg_color;
    uint32_t text_color;
    uint32_t text_muted;
    uint32_t h1_color;
    uint32_t h2_color;
    uint32_t h3_color;
    uint32_t code_bg;
    uint32_t code_border;
    uint32_t code_text;
    uint32_t quote_bar;
    uint32_t quote_text;
    uint32_t list_bullet;
    uint32_t hrule_color;
    uint32_t scrollbar_track;
    uint32_t scrollbar_thumb;
    uint32_t header_bg;
    uint32_t header_text;
    uint32_t accent;
} Theme;

void theme_init(void);
ThemeType theme_get_current_type(void);
const Theme *theme_get_current(void);
void theme_set(ThemeType type);
void theme_cycle_next(void);

#ifdef __cplusplus
}
#endif

#endif /* THEME_ENGINE_H */
