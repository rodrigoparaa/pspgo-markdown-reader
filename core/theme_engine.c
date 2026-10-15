#include "theme_engine.h"

static Theme s_themes[THEME_COUNT] = {
    [THEME_DARK] = {
        .name = "Dark Cyberpunk",
        .bg_color         = COLOR_RGB(15, 15, 20),      /* #0F0F14 */
        .text_color       = COLOR_RGB(224, 230, 237),   /* #E0E6ED */
        .text_muted       = COLOR_RGB(120, 130, 145),   /* #788291 */
        .h1_color         = COLOR_RGB(32, 170, 221),    /* #20AADD Cian */
        .h2_color         = COLOR_RGB(80, 200, 240),    /* #50C8F0 */
        .h3_color         = COLOR_RGB(130, 215, 245),   /* #82D7F5 */
        .code_bg          = COLOR_RGB(26, 24, 34),      /* #1A1822 */
        .code_border      = COLOR_RGB(50, 48, 65),      /* #323041 */
        .code_text        = COLOR_RGB(85, 204, 102),    /* #55CC66 Verde Terminal */
        .quote_bar        = COLOR_RGB(32, 170, 221),    /* #20AADD */
        .quote_text       = COLOR_RGB(170, 180, 195),   /* #AAB4C3 */
        .list_bullet      = COLOR_RGB(240, 176, 32),    /* #F0B020 Ámbar */
        .hrule_color      = COLOR_RGB(45, 45, 60),      /* #2D2D3C */
        .scrollbar_track  = COLOR_RGB(25, 25, 32),      /* #191920 */
        .scrollbar_thumb  = COLOR_RGB(60, 70, 90),      /* #3C465A */
        .header_bg        = COLOR_RGB(20, 20, 28),      /* #14141C */
        .header_text      = COLOR_RGB(200, 210, 225),   /* #C8D2E1 */
        .accent           = COLOR_RGB(32, 170, 221)     /* #20AADD */
    },
    [THEME_SEPIA] = {
        .name = "Sepia Paper",
        .bg_color         = COLOR_RGB(244, 236, 216),   /* #F4ECD8 Pergamino */
        .text_color       = COLOR_RGB(62, 49, 35),      /* #3E3123 Café oscuro */
        .text_muted       = COLOR_RGB(130, 115, 95),    /* #82735F */
        .h1_color         = COLOR_RGB(139, 37, 0),      /* #8B2500 Terracota rojizo */
        .h2_color         = COLOR_RGB(170, 60, 20),     /* #AA3C14 */
        .h3_color         = COLOR_RGB(110, 50, 20),     /* #6E3214 */
        .code_bg          = COLOR_RGB(230, 220, 190),   /* #E6DCBE */
        .code_border      = COLOR_RGB(200, 190, 160),   /* #C8BEA0 */
        .code_text        = COLOR_RGB(35, 35, 35),      /* #232323 */
        .quote_bar        = COLOR_RGB(139, 37, 0),      /* #8B2500 */
        .quote_text       = COLOR_RGB(90, 75, 60),      /* #5A4B3C */
        .list_bullet      = COLOR_RGB(139, 37, 0),      /* #8B2500 */
        .hrule_color      = COLOR_RGB(210, 200, 175),   /* #D2C8AF */
        .scrollbar_track  = COLOR_RGB(235, 225, 205),   /* #EBE1CD */
        .scrollbar_thumb  = COLOR_RGB(170, 155, 130),   /* #AA9B82 */
        .header_bg        = COLOR_RGB(235, 225, 200),   /* #EBE1C8 */
        .header_text      = COLOR_RGB(70, 55, 40),      /* #463728 */
        .accent           = COLOR_RGB(139, 37, 0)       /* #8B2500 */
    },
    [THEME_OLED] = {
        .name = "OLED High-Contrast",
        .bg_color         = COLOR_RGB(0, 0, 0),         /* #000000 Negro puro */
        .text_color       = COLOR_RGB(255, 255, 255),   /* #FFFFFF Blanco puro */
        .text_muted       = COLOR_RGB(140, 140, 140),   /* #8C8C8C */
        .h1_color         = COLOR_RGB(240, 176, 32),    /* #F0B020 Ámbar puro */
        .h2_color         = COLOR_RGB(255, 200, 70),    /* #FFC846 */
        .h3_color         = COLOR_RGB(255, 220, 120),   /* #FFDC78 */
        .code_bg          = COLOR_RGB(16, 16, 16),      /* #101010 */
        .code_border      = COLOR_RGB(60, 60, 60),      /* #3C3C3C */
        .code_text        = COLOR_RGB(220, 220, 220),   /* #DCDCDC */
        .quote_bar        = COLOR_RGB(240, 176, 32),    /* #F0B020 */
        .quote_text       = COLOR_RGB(190, 190, 190),   /* #BEBEBE */
        .list_bullet      = COLOR_RGB(240, 176, 32),    /* #F0B020 */
        .hrule_color      = COLOR_RGB(60, 60, 60),      /* #3C3C3C */
        .scrollbar_track  = COLOR_RGB(20, 20, 20),      /* #141414 */
        .scrollbar_thumb  = COLOR_RGB(100, 100, 100),   /* #646464 */
        .header_bg        = COLOR_RGB(10, 10, 10),      /* #0A0A0A */
        .header_text      = COLOR_RGB(255, 255, 255),   /* #FFFFFF */
        .accent           = COLOR_RGB(240, 176, 32)     /* #F0B020 */
    }
};

static ThemeType s_current_theme = THEME_DARK;

void theme_init(void) {
    s_current_theme = THEME_DARK;
}

ThemeType theme_get_current_type(void) {
    return s_current_theme;
}

const Theme *theme_get_current(void) {
    return &s_themes[s_current_theme];
}

void theme_set(ThemeType type) {
    if (type >= 0 && type < THEME_COUNT) {
        s_current_theme = type;
    }
}

void theme_cycle_next(void) {
    s_current_theme = (ThemeType)((s_current_theme + 1) % THEME_COUNT);
}
