#ifndef SCREENS_H
#define SCREENS_H

#include "../core/app_state.h"

#ifdef __cplusplus
extern "C" {
#endif

void screen_render_explorer(const AppState *state);
void screen_render_reader(const AppState *state);
void screen_render_help(const AppState *state);

#ifdef __cplusplus
}
#endif

#endif /* SCREENS_H */
