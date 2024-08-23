#ifndef BK_COMMON_H
#define	BK_COMMON_H

#ifndef	MEM_FILE_H
#include "mem_file.h"
#endif

typedef struct  __attribute__((packed)) tagimgImage {
	uint16_t	i_width;
	uint16_t	i_height;
	uint16_t	i_rowbytes;
	uint32_t	i_bits;
} imgImage;

typedef struct  __attribute__((packed)) taghost_imgImage {
	uint16_t	i_width;
	uint16_t	i_height;
	uint16_t	i_rowbytes;
	uint8_t	*p_bits;
} host_imgImage;

#define	RIC_MAX_APPS		8

typedef struct tagROM_info_common {
	int	ric_rom_version;

	int	ric_num_apps;
	char	*ric_app_names[RIC_MAX_APPS];

	uint16_t	ric_appID[RIC_MAX_APPS];
	uint16_t	ric_appRev[RIC_MAX_APPS];
	uint16_t	ric_appFlags[RIC_MAX_APPS];
	uint32_t	ric_p_appEntry[RIC_MAX_APPS];
	uint32_t	ric_p_appKeyTrans[RIC_MAX_APPS];

	uint32_t	ric_p_appIconSize[RIC_MAX_APPS];
	uint32_t	ric_p_appIconDraw[RIC_MAX_APPS];

	uint32_t	ric_p_appIcon[RIC_MAX_APPS];

	uint32_t	ric_p_biosBookInfo[RIC_MAX_APPS];

	uint32_t	ric_p_appIrqhandler[RIC_MAX_APPS];
	uint32_t	ric_p_appDrawChar[RIC_MAX_APPS];

	uint32_t	ric_p_app_red_legend[RIC_MAX_APPS];
	uint32_t	ric_p_app_green_legend[RIC_MAX_APPS];
	uint32_t	ric_p_app_yellow_legend[RIC_MAX_APPS];
	uint32_t	ric_p_app_blue_legend[RIC_MAX_APPS];

	char	*ric_red_legends[RIC_MAX_APPS];
	char	*ric_green_legends[RIC_MAX_APPS];
	char	*ric_yellow_legends[RIC_MAX_APPS];
	char	*ric_blue_legends[RIC_MAX_APPS];

	host_imgImage	*ric_app_icons[RIC_MAX_APPS];

} ROM_info_common;

#define	RIC_MAX_CHIPSELS	16

typedef struct tagROM_layout_info_common {
	int	rli_max_chipsels;
	int	rli_num_chipsels;
	uint8_t	rli_chipsel_selects[RIC_MAX_CHIPSELS];
	uint32_t	rli_chipsel_baseAddrs[RIC_MAX_CHIPSELS];
	uint32_t	rli_chipsel_byteCounts[RIC_MAX_CHIPSELS];
	uint16_t	rli_chipsel_checksums[RIC_MAX_CHIPSELS];
	uint8_t	rli_chipsel_checksums_good[RIC_MAX_CHIPSELS];
	int	rli_chipsel_all_checksums_good;
} ROM_layout_info_common;

#define	MAX_APPNAME_LEN_COMMON	256		// arbitrary/much larger than reality
#define	MAX_KEY_LEGEND_COMMON	64		// arbitrary/much larger than reality

extern int	read_string(MEMF *mf, loff_t offset, char *p_out, int maxlen);
extern int	read_imgImage(MEMF *mf, loff_t offset, imgImage *p_out);
extern int	read_host_imgImage(MEMF *mf, loff_t offset, host_imgImage *p_out);
extern int	draw_host_imgImage(host_imgImage *p_image, int ntabs_indent);

extern int	load_rom_info_common(MEMF *mf, ROM_info_common *ric);
#endif // BK_COMMON_H
