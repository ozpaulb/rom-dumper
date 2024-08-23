#ifndef BK_STRUCTS_V1_H
#define	BK_STRUCTS_V1_H

#ifndef BK_COMMON_H
#include "bk_common.h"
#endif
#ifndef	MEM_FILE_H
#include "mem_file.h"
#endif

//
// HostCardInfo_v1 struct is at the start of the ROM image
//
#define	CARDINFO_MAGIC_ID_V1		"Bookman1"
#define	CARDINFO_MAGIC_ID_LEN_V1	(sizeof(CARDINFO_MAGIC_ID_V1)-1)

typedef struct  __attribute__((packed)) tagHostCardInfo_v1 {
	uint8_t	ci_cardMagic[CARDINFO_MAGIC_ID_LEN_V1];
	uint8_t	ci_cardIsPlatformRom;
	uint8_t	ci_cardBiosRevision[2];	// minor, major
	uint16_t	ci_cardID;
	uint8_t	ci_romSize;
	uint8_t	ci_numApps;
	uint32_t	ci_cardRamSize;

#define	CI_RESERVED_BYTES_V1	13
	uint8_t	ci_reserved[CI_RESERVED_BYTES_V1];
} HostCardInfo_v1;

//
// cardChecksumInfo immediately follows HostCardInfo
//
#define	CCI_MAX_CHIPS_V1	8
typedef struct  __attribute__((packed)) tagcardChecksumInfo_v1 {
	uint8_t	csi_chipSels[CCI_MAX_CHIPS_V1];
	uint32_t	csi_baseAddrs[CCI_MAX_CHIPS_V1];
	uint32_t	csi_byteCounts[CCI_MAX_CHIPS_V1];
	uint16_t	csi_checksums[CCI_MAX_CHIPS_V1];
	uint16_t	csi_checkfix;
} cardChecksumInfo_v1;


//
// array of up to 2 appInfo structs immediately follows cardChecksumInfo
//
#define	AI_MAX_APPS_V1		2
typedef struct  __attribute__((packed)) tagappInfo_v1 {
	uint16_t	ai_entry;
	uint16_t	ai_keytrans;
	uint16_t	ai_icon;
#define	AI_MAX_APPNAME_LEN_V1	32
	uint8_t	ai_appname[AI_MAX_APPNAME_LEN_V1];
	uint16_t	ai_appID;
	uint16_t	ai_appRev;
	uint16_t	ai_appFlags;

	uint16_t	ai_irqhandler;
	uint16_t	ai_drawchar;

	uint16_t	ai_red_legend;
	uint16_t	ai_green_legend;
	uint16_t	ai_yellow_legend;
	uint16_t	ai_blue_legend;

#define	AI_RESERVED_BYTES_V1	8
	uint8_t	ai_reserved[AI_RESERVED_BYTES_V1];
} appInfo_v1;

extern int	is_bookman_image_v1(MEMF *mf);
extern int	load_rom_info_v1(MEMF *mf, ROM_info_common *rci, ROM_layout_info_common *rli, int only_chip_layout);

extern int	read_HostCardInfo_v1(MEMF *mf, int quiet, HostCardInfo_v1 *p_out);
extern int	read_cardChecksumInfo_v1(MEMF *mf, cardChecksumInfo_v1 *p_out);
extern int	read_appInfo_v1(MEMF *mf, appInfo_v1 *p_out);

#endif // BK_STRUCTS_V1_H
