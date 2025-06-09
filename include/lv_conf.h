#ifndef LV_CONF_H
#define LV_CONF_H

/* Set the default color depth (1, 8, 16, 32) */
#define LV_COLOR_DEPTH 16

/* Enable more complex drawing routines */
#define LV_DRAW_COMPLEX 1

/* Enable anti-aliasing (lines, and radiuses will be smoothed) */
#define LV_ANTIALIAS 1

/* Enable more objects */
#define LV_USE_LABEL 1
#define LV_USE_BTN 1
#define LV_USE_BTNMATRIX 1
#define LV_USE_CANVAS 1

/* Enable more fonts */
#define LV_FONT_MONTSERRAT_12 1
#define LV_FONT_MONTSERRAT_14 1
#define LV_FONT_MONTSERRAT_16 1
#define LV_FONT_MONTSERRAT_20 1
#define LV_FONT_MONTSERRAT_24 1
#define LV_FONT_MONTSERRAT_32 1
#define LV_FONT_MONTSERRAT_48 1

/* Enable themes */
#define LV_USE_THEME_DEFAULT 1

/* Enable log module */
#define LV_USE_LOG 1

/* Memory settings */
#define LV_MEM_SIZE (32U * 1024U)

/* Enable if you want to see the memory usage in the lower right corner */
#define LV_USE_MEM_MONITOR 1

/* Enable if you want to see the FPS and CPU usage in the lower right corner */
#define LV_USE_PERF_MONITOR 1

#endif /*LV_CONF_H*/
