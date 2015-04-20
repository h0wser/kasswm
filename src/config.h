#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

typedef struct CONFIG {
	/* Window border settings */	 
	uint32_t border_normal_color;
	uint32_t border_focused_color;
	uint16_t border_width;
} CONFIG;

const CONFIG DEFALT_CONFIG = {
	/* Window border settings */	 
	.border_normal_color = 0x222222,
	.border_focused_color = 0xaaaaaa,
	.border_width = 3
};

CONFIG get_default_config(void)
{
	return DEFALT_CONFIG;
}

#endif /* CONFIG_H */
