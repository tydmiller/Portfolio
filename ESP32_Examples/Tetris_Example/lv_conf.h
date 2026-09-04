#ifndef LV_CONF_H
#define LV_CONF_H

#define LV_COLOR_DEPTH     16
#define LV_COLOR_16_SWAP   0
#define LV_HOR_RES_MAX     172
#define LV_VER_RES_MAX     320
#define LV_USE_ARGB8888    0
#define LV_FONT_DEFAULT    &lv_font_montserrat_14

#define LV_TICK_CUSTOM     1
#define LV_TICK_CUSTOM_INCLUDE <Arduino.h>
#define LV_TICK_CUSTOM_SYS_TIME_EXPR (millis())

#define LV_USE_PERF_MONITOR 0
#define LV_USE_LOG         0
#define LV_USE_GPU_STM32_DMA 0

#endif /*LV_CONF_H*/