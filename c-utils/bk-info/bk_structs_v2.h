#ifndef BK_STRUCTS_V2_H
#define	BK_STRUCTS_V2_H

#ifndef BK_COMMON_H
#include "bk_common.h"
#endif
#ifndef	MEM_FILE_H
#include "mem_file.h"
#endif


#ifndef	U24_type_defined
typedef uint8_t	uint24_t[3];
#endif

#define	CARDINFO_MAGIC_ID_V2		"BOOKMAN card 01"
#define	CARDINFO_MAGIC_ID_LEN_V2	(sizeof(CARDINFO_MAGIC_ID_V2))	// inclues NULL

typedef struct  __attribute__((packed)) tagHostCardInfo_v2 {
	uint8_t	ci_cardMagic[CARDINFO_MAGIC_ID_LEN_V2];
	uint24_t	p_appInfo;
	uint16_t	v_150;
	uint8_t		ci_cs_size;
	uint16_t	ci_cardID;
	uint16_t	ci_flags;
	uint24_t	p_biosInfo;
	uint8_t	x_unknown_1_00[5];
	uint8_t	x_unknown_2_ff[6];
	uint8_t	datestr[8];
	uint16_t	datestr_fixup;
	uint8_t	timestr[8];
	uint16_t	timestr_fixup;
#define	CCI_MAX_CHIPS_V2	6
	uint8_t	csi_chipSels[CCI_MAX_CHIPS_V2];
	uint32_t	csi_baseAddrs[CCI_MAX_CHIPS_V2];
	uint32_t	csi_byteCounts[CCI_MAX_CHIPS_V2];
	uint16_t	csi_checksums[CCI_MAX_CHIPS_V2];
	uint16_t	csi_checkfix;
} HostCardInfo_v2;

#define	AI_MAX_APPS_V2		RIC_MAX_APPS	// arbitrary
typedef struct  __attribute__((packed)) tagappInfo_v2 {
	uint24_t	ai_p_next_app;
#define	AI_MAGICVAL_V2	0xdead
	uint16_t	ai_magic;
	uint24_t	ai_p_appname;
	uint16_t	ai_appID;
	uint16_t	ai_appFlags;

	uint24_t	ai_p_appEntry;
	uint24_t	ai_p_appKeyTrans;
	uint24_t	ai_p_appDrawChar;
	uint24_t	ai_p_appIconSize;
	uint24_t	ai_p_appIconDraw;

	uint24_t	ai_p_appIcon;
	uint24_t	ai_p_biosBookInfo;
	uint24_t	ai_p_red_legend;
	uint24_t	ai_p_green_legend;
	uint24_t	ai_p_yellow_legend;
	uint24_t	ai_p_blue_legend;

#define	AI_RESERVED_BYTES_V2	8
	uint8_t	ai_reserved[AI_RESERVED_BYTES_V2];
} appInfo_v2;

extern int	is_bookman_image_v2(MEMF *mf, int *p_need_a20_flip);
extern int	load_rom_info_v2(MEMF *mf, ROM_info_common *rci, ROM_layout_info_common *rli, int only_chip_layout);
extern int	read_HostCardInfo_v2(MEMF *mf, int quiet, HostCardInfo_v2 *p_out, int *p_need_a20_flip);
extern int	read_appInfo_v2(MEMF *mf, loff_t offset, appInfo_v2 *p_out);


#endif // BK_STRUCTS_V2_H
