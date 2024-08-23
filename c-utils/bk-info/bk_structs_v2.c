#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <endian.h>

#include "my_endian.h"
#include "bk_structs_v2.h"
#include "bk_common.h"

static uint32_t	_read24(uint24_t *p_val_in);

int
is_bookman_image_v2(MEMF *mf)
{
	HostCardInfo_v2	hci;

	if (!mf) return 1;

	if (read_HostCardInfo_v2(mf, 1, &hci)) return 0;

	return 1;
}
#define	_get24_fileoffs(p, filesize)	(_read24(p) & (filesize-1))

int
load_rom_info_v2(MEMF *mf, ROM_info_common *ric, ROM_layout_info_common *rli, int only_chip_layout)
{
	HostCardInfo_v2	hci;
	long	filesize;
	appInfo_v2	ai;	// FIXME: hard-coded limit to a single app for now
	long	ai_offs;
	char	appname_buf[MAX_APPNAME_LEN_COMMON+1];
	int	retval = 1;
	int	app_idx = 0;
	int	i;

	if (!mf) goto exit_err;

	filesize = mf_filesize(mf);

	memset((void *)ric, 0, sizeof(*ric));

	ric->ric_rom_version = 2;

	if (read_HostCardInfo_v2(mf, 0, &hci)) goto exit_err;

	if (rli) {
		rli->rli_max_chipsels = ((CCI_MAX_CHIPS_V2 < RIC_MAX_CHIPSELS) ? CCI_MAX_CHIPS_V2 : RIC_MAX_CHIPSELS);
		memset((void *)rli->rli_chipsel_selects, 0xff, sizeof(rli->rli_chipsel_selects));
		rli->rli_num_chipsels = rli->rli_max_chipsels;
		for (i = 0; i < rli->rli_max_chipsels; i++) {
			rli->rli_chipsel_selects[i] = hci.csi_chipSels[i];

			rli->rli_chipsel_baseAddrs[i] = hci.csi_baseAddrs[i];
			rli->rli_chipsel_byteCounts[i] = hci.csi_byteCounts[i];
			rli->rli_chipsel_checksums[i] = hci.csi_checksums[i];

			if ((rli->rli_num_chipsels == rli->rli_max_chipsels) && !rli->rli_chipsel_byteCounts[i]) rli->rli_num_chipsels = i;
		}
	}
	if (only_chip_layout) return 0;


	ai_offs = _get24_fileoffs(&hci.p_appInfo, filesize);

	if (read_appInfo_v2(mf, ai_offs, &ai)) goto exit_err;

	appname_buf[0] = '\0';
	if (ai.ai_p_appname && read_string(mf, (loff_t)_get24_fileoffs(&ai.ai_p_appname, filesize), appname_buf, MAX_APPNAME_LEN_COMMON)) goto exit_err;

	ric->ric_num_apps = 1;	// FIXME

	app_idx = 0;

	ric->ric_app_names[app_idx] = strdup(appname_buf ?: "");

	ric->ric_appID[app_idx] = ai.ai_appID;
	ric->ric_appFlags[app_idx] = ai.ai_appFlags;

	ric->ric_p_appEntry[app_idx] = _get24_fileoffs(&ai.ai_p_appEntry, 0);
	ric->ric_p_appKeyTrans[app_idx] = _get24_fileoffs(&ai.ai_p_appKeyTrans, 0);
	ric->ric_p_appDrawChar[app_idx] = _get24_fileoffs(&ai.ai_p_appDrawChar, 0);
	ric->ric_p_appIconSize[app_idx] = _get24_fileoffs(&ai.ai_p_appIconSize, 0);
	ric->ric_p_appIconDraw[app_idx] = _get24_fileoffs(&ai.ai_p_appIconDraw, 0);

	ric->ric_p_appIcon[app_idx] = _get24_fileoffs(&ai.ai_p_appIcon, 0);

	ric->ric_p_biosBookInfo[app_idx] = _get24_fileoffs(&ai.ai_p_biosBookInfo, 0);

	ric->ric_p_app_red_legend[app_idx] = _get24_fileoffs(&ai.ai_p_red_legend, 0);
	ric->ric_p_app_green_legend[app_idx] = _get24_fileoffs(&ai.ai_p_green_legend, 0);
	ric->ric_p_app_yellow_legend[app_idx] = _get24_fileoffs(&ai.ai_p_yellow_legend, 0);
	ric->ric_p_app_blue_legend[app_idx] = _get24_fileoffs(&ai.ai_p_blue_legend, 0);


	goto exit_ok;

exit_err:
	retval = 1;
	goto exit_common;

exit_ok:
	retval = 0;
	// fallthru
exit_common:

	return retval;
}


static uint32_t
_read24(uint24_t *p_val_in)
{
	uint8_t	*p_val = (uint8_t *)p_val_in;

	return (p_val[0] | (p_val[1] << 8) | (p_val[2] << 16));
}

int
read_HostCardInfo_v2(MEMF *mf, int quiet, HostCardInfo_v2 *p_out)
{
	long	filesize;
	long	hdrpos;
	int	i;

	if (!p_out) return 1;

	filesize = mf_filesize(mf);

	hdrpos = (filesize - 128);

	if (mf_seek(mf, hdrpos) != 0) return 1;

	if (mf_read(mf, (void *)p_out, sizeof(*p_out)) != sizeof(*p_out)) return 1;

	if (memcmp(p_out->ci_cardMagic, CARDINFO_MAGIC_ID_V2, CARDINFO_MAGIC_ID_LEN_V2)) {
		if (!quiet) fprintf(stderr, "hci: bad magic!\n");
		return 1;
	}

	endian_fix_LE_val_to_host(&p_out->p_appInfo);
	endian_fix_LE_val_to_host(&p_out->v_150);
	endian_fix_LE_val_to_host(&p_out->ci_cardID);
	endian_fix_LE_val_to_host(&p_out->ci_flags);
	endian_fix_LE_val_to_host(&p_out->p_biosInfo);
	endian_fix_LE_val_to_host(&p_out->datestr_fixup);
	endian_fix_LE_val_to_host(&p_out->timestr_fixup);
	endian_fix_LE_val_to_host(&p_out->p_appInfo);
	endian_fix_LE_val_to_host(&p_out->p_appInfo);
	endian_fix_LE_val_to_host(&p_out->p_appInfo);

	for (i = 0; i < CCI_MAX_CHIPS_V2; i++) {
		endian_fix_LE_val_to_host(&p_out->csi_baseAddrs[i]);
		endian_fix_LE_val_to_host(&p_out->csi_baseAddrs[i]);
		endian_fix_LE_val_to_host(&p_out->csi_checksums[i]);
	}

	return 0;
}

int
read_appInfo_v2(MEMF *mf, loff_t offset, appInfo_v2 *p_out)
{
	if (!p_out) return 1;

	if (mf_seek(mf, offset) != 0) return 1;

	if (mf_read(mf, (void *)p_out, sizeof(*p_out)) != sizeof(*p_out)) return 1;

	endian_fix_LE_val_to_host(&p_out->ai_p_next_app);
	endian_fix_LE_val_to_host(&p_out->ai_magic);
	if (p_out->ai_magic != AI_MAGICVAL_V2) {
		fprintf(stderr, "ERROR: bad app magic!\n");
		return 1;
	}

	endian_fix_LE_val_to_host(&p_out->ai_p_appname);

	endian_fix_LE_val_to_host(&p_out->ai_appID);
	endian_fix_LE_val_to_host(&p_out->ai_appFlags);

	endian_fix_LE_val_to_host(&p_out->ai_p_appEntry);
	endian_fix_LE_val_to_host(&p_out->ai_p_appKeyTrans);
	endian_fix_LE_val_to_host(&p_out->ai_p_appDrawChar);
	endian_fix_LE_val_to_host(&p_out->ai_p_appIconSize);
	endian_fix_LE_val_to_host(&p_out->ai_p_appIconDraw);

	endian_fix_LE_val_to_host(&p_out->ai_p_appIcon);
	endian_fix_LE_val_to_host(&p_out->ai_p_biosBookInfo);
	endian_fix_LE_val_to_host(&p_out->ai_p_red_legend);
	endian_fix_LE_val_to_host(&p_out->ai_p_green_legend);
	endian_fix_LE_val_to_host(&p_out->ai_p_yellow_legend);
	endian_fix_LE_val_to_host(&p_out->ai_p_blue_legend);

	return 0;
}
