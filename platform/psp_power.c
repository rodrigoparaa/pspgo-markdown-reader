#include "psp_power.h"
#include <stdio.h>
#include <time.h>

#if defined(__PSP__) || defined(PSP)
#include <psppower.h>
#include <psprtc.h>
#include <psputility_sysparam.h>

void psp_power_init(void) {
    /* Inicialización de energía */
    scePowerTick(PSP_POWER_TICK_ALL);
}

void psp_power_tick(void) {
    /* Mantener consola despierta durante lectura prolongada */
    scePowerTick(PSP_POWER_TICK_ALL);
}

int psp_power_get_battery_percent(void) {
    int pct = scePowerGetBatteryLifePercent();
    if (pct < 0) return 0;
    if (pct > 100) return 100;
    return pct;
}

bool psp_power_is_charging(void) {
    return scePowerIsBatteryCharging() ? true : false;
}

void psp_power_get_time_str(char *out_buf, size_t buf_len) {
    if (!out_buf || buf_len < 6) return;
    ScePspDateTime t;
    if (sceRtcGetCurrentClockLocalTime(&t) == 0) {
        int time_format = PSP_SYSTEMPARAM_TIME_FORMAT_24HR;
        sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_TIME_FORMAT, &time_format);

        if (time_format == PSP_SYSTEMPARAM_TIME_FORMAT_12HR) {
            int h = t.hour;
            const char *ampm = (h >= 12) ? "PM" : "AM";
            if (h == 0) h = 12;
            else if (h > 12) h -= 12;
            snprintf(out_buf, buf_len, "%d:%02d %s", h, t.minute, ampm);
        } else {
            snprintf(out_buf, buf_len, "%02d:%02d", t.hour, t.minute);
        }
    } else {
        snprintf(out_buf, buf_len, "--:--");
    }
}

#else
/* Stub host para macOS */
void psp_power_init(void) {}
void psp_power_tick(void) {}
int psp_power_get_battery_percent(void) { return 100; }
bool psp_power_is_charging(void) { return false; }
void psp_power_get_time_str(char *out_buf, size_t buf_len) {
    time_t now = time(NULL);
    struct tm *tm = localtime(&now);
    if (tm) {
        snprintf(out_buf, buf_len, "%02d:%02d", tm->tm_hour, tm->tm_min);
    } else {
        snprintf(out_buf, buf_len, "12:00");
    }
}
#endif
