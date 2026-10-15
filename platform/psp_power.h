#ifndef PSP_POWER_H
#define PSP_POWER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

void psp_power_init(void);
void psp_power_tick(void);

int psp_power_get_battery_percent(void);
bool psp_power_is_charging(void);

/* Obtiene hora actual formateada en "HH:MM" */
void psp_power_get_time_str(char *out_buf, size_t buf_len);

#ifdef __cplusplus
}
#endif

#endif /* PSP_POWER_H */
