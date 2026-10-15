#include "scroll_physics.h"
#include <math.h>
#include <stdlib.h>

#define STICK_DEADZONE      22
#define FRICTION            0.88f
#define PAGE_OVERLAP_PX     32

static float get_max_scroll_y(const ScrollState *s) {
    float max_y = (float)(s->total_height - s->viewport_height);
    return (max_y < 0.0f) ? 0.0f : max_y;
}

void scroll_init(ScrollState *s, int total_height, int viewport_height) {
    if (!s) return;
    s->current_y = 0.0f;
    s->target_y = 0.0f;
    s->velocity_y = 0.0f;
    s->total_height = total_height;
    s->viewport_height = (viewport_height > 0) ? viewport_height : 272;
    s->is_animating = false;
}

void scroll_set_bounds(ScrollState *s, int total_height, int viewport_height) {
    if (!s) return;
    s->total_height = total_height;
    if (viewport_height > 0) {
        s->viewport_height = viewport_height;
    }
    float max_y = get_max_scroll_y(s);
    if (s->current_y > max_y) s->current_y = max_y;
    if (s->target_y > max_y) s->target_y = max_y;
}

void scroll_apply_analog(ScrollState *s, int analog_y) {
    if (!s) return;

    if (abs(analog_y) < STICK_DEADZONE) {
        return;
    }

    /* Interrumpir animación de página si el usuario mueve el stick */
    s->is_animating = false;

    /* Aceleración proporcional cuadrática para control fino a baja inclinación */
    float sign = (analog_y > 0) ? 1.0f : -1.0f;
    float norm = ((float)abs(analog_y) - STICK_DEADZONE) / (128.0f - STICK_DEADZONE);
    if (norm > 1.0f) norm = 1.0f;

    float speed = sign * (norm * norm * 14.0f + norm * 2.0f);
    s->velocity_y = speed;
}

void scroll_page_down(ScrollState *s) {
    if (!s) return;
    int page_step = s->viewport_height - PAGE_OVERLAP_PX;
    if (page_step < 50) page_step = 50;

    float base = s->is_animating ? s->target_y : s->current_y;
    s->target_y = base + (float)page_step;
    float max_y = get_max_scroll_y(s);
    if (s->target_y > max_y) s->target_y = max_y;

    s->velocity_y = 0.0f;
    s->is_animating = true;
}

void scroll_page_up(ScrollState *s) {
    if (!s) return;
    int page_step = s->viewport_height - PAGE_OVERLAP_PX;
    if (page_step < 50) page_step = 50;

    float base = s->is_animating ? s->target_y : s->current_y;
    s->target_y = base - (float)page_step;
    if (s->target_y < 0.0f) s->target_y = 0.0f;

    s->velocity_y = 0.0f;
    s->is_animating = true;
}

void scroll_line_up(ScrollState *s) {
    if (!s) return;
    s->is_animating = false;
    s->current_y -= 16.0f;
    if (s->current_y < 0.0f) s->current_y = 0.0f;
    s->target_y = s->current_y;
    s->velocity_y = 0.0f;
}

void scroll_line_down(ScrollState *s) {
    if (!s) return;
    s->is_animating = false;
    s->current_y += 16.0f;
    float max_y = get_max_scroll_y(s);
    if (s->current_y > max_y) s->current_y = max_y;
    s->target_y = s->current_y;
    s->velocity_y = 0.0f;
}

void scroll_update(ScrollState *s) {
    if (!s) return;

    float max_y = get_max_scroll_y(s);

    if (s->is_animating) {
        /* Interpolación suave hacia target_y */
        float diff = s->target_y - s->current_y;
        if (fabsf(diff) < 0.5f) {
            s->current_y = s->target_y;
            s->is_animating = false;
        } else {
            s->current_y += diff * 0.28f;
        }
    } else {
        /* Física inercial */
        s->current_y += s->velocity_y;
        s->velocity_y *= FRICTION;
        if (fabsf(s->velocity_y) < 0.05f) {
            s->velocity_y = 0.0f;
        }
    }

    /* Clamping estricto */
    if (s->current_y < 0.0f) {
        s->current_y = 0.0f;
        s->velocity_y = 0.0f;
    } else if (s->current_y > max_y) {
        s->current_y = max_y;
        s->velocity_y = 0.0f;
    }
}

int scroll_get_percent(const ScrollState *s) {
    if (!s) return 0;
    float max_y = get_max_scroll_y(s);
    if (max_y <= 0.0f) return 100;

    int pct = (int)((s->current_y / max_y) * 100.0f + 0.5f);
    if (pct < 0) pct = 0;
    if (pct > 100) pct = 100;
    return pct;
}

int scroll_get_render_y(const ScrollState *s) {
    if (!s) return 0;
    return (int)(s->current_y + 0.5f);
}
