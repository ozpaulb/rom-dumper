#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "my_endian.h"

void
__endian_swap(uint8_t *p_val, size_t sz_val)
{
	uint32_t	val32;

	switch(sz_val) {
	case 2:
		val32 = ((p_val[0] << 8) | p_val[1]);
		p_val[0] = (val32 & 0xff);
		p_val[1] = ((val32 >> 8) & 0xff);
		break;
	case 3:
		val32 = ((p_val[0] << 16) | (p_val[1] << 8) | p_val[2]);
		p_val[0] = (val32 & 0xff);
		p_val[1] = ((val32 >> 8) & 0xff);
		p_val[2] = ((val32 >> 16) & 0xff);
		break;
	case 4:
		val32 = ((p_val[0] << 24) | (p_val[1] << 16) | (p_val[2] << 8) | p_val[3]);
		p_val[0] = (val32 & 0xff);
		p_val[1] = ((val32 >> 8) & 0xff);
		p_val[2] = ((val32 >> 16) & 0xff);
		p_val[3] = ((val32 >> 24) & 0xff);
		break;
	default:
		break;
	}
}

