#ifndef SCROLL_PHYSICS_H
#define SCROLL_PHYSICS_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    float current_y;        /* Posición actual del scroll en píxeles */
    float target_y;         /* Destino deseado para transiciones suaves de página */
    float velocity_y;       /* Velocidad inercial actual (px/frame) */
    int total_height;       /* Altura total del documento maquetado */
    int viewport_height;    /* Altura útil visible de la pantalla */
    bool is_animating;      /* Indica si hay animación de salto de página activa */
} ScrollState;

/* Inicializa el estado del scroll */
void scroll_init(ScrollState *s, int total_height, int viewport_height);

/* Actualiza límites del documento tras cargar un nuevo archivo */
void scroll_set_bounds(ScrollState *s, int total_height, int viewport_height);

/* Aplica entrada analógica del stick (-128 a 127, con zona muerta interna) */
void scroll_apply_analog(ScrollState *s, int analog_y);

/* Salto de página siguiente (Gatillo R) */
void scroll_page_down(ScrollState *s);

/* Salto de página anterior (Gatillo L) */
void scroll_page_up(ScrollState *s);

/* Desplazamiento paso a paso de línea (Cruceta arriba/abajo) */
void scroll_line_up(ScrollState *s);
void scroll_line_down(ScrollState *s);

/* Bucle de actualización física llamado cada fotograma (60 Hz) */
void scroll_update(ScrollState *s);

/* Devuelve el porcentaje de lectura actual [0..100] */
int scroll_get_percent(const ScrollState *s);

/* Devuelve la posición Y entera redondeada para el renderizado */
int scroll_get_render_y(const ScrollState *s);

#ifdef __cplusplus
}
#endif

#endif /* SCROLL_PHYSICS_H */
