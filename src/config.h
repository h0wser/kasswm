#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

typedef struct CONFIG {
	/* Window border settings */	 
	uint32_t border_normal_color;
	uint32_t border_focused_color;
	uint16_t border_width;
} CONFIG;

extern CONFIG DEFAULT_CONFIG;

#endif /* CONFIG_H */
