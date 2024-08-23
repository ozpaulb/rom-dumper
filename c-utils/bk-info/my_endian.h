#ifndef MY_ENDIAN_H
#define MY_ENDIAN_H

#include <endian.h>

extern	void	__endian_swap(uint8_t *p_val, size_t sz_val);

#if __BYTE_ORDER != __LITTLE_ENDIAN
#define	endian_fix_BE_val_to_host(v)
#define	endian_fix_LE_val_to_host(v)	__endian_swap((uint8_t *)(v), sizeof(*(v)))
#else
#define	endian_fix_BE_val_to_host(v)	__endian_swap((uint8_t *)(v), sizeof(*(v)))
#define	endian_fix_LE_val_to_host(v)
#endif

#endif // MY_ENDIAN_H
