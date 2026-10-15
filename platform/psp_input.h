#ifndef PSP_INPUT_H
#define PSP_INPUT_H

#include <stdint.h>
#include <stdbool.h>
#include "../core/app_state.h"

#ifdef __cplusplus
extern "C" {
#endif

void psp_input_init(void);

/* Lee controles, procesa gestos, combos y actualiza el estado de la app */
void psp_input_poll(AppState *state, uint32_t now_ms);

#ifdef __cplusplus
}
#endif

#endif /* PSP_INPUT_H */
