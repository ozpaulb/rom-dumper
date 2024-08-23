#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <endian.h>

#include "bk_structs_v1.h"
#include "bk_common.h"
#include "my_endian.h"

static int	is_known_bad_key_legends_V1(appInfo_v1 *ai);

int
is_bookman_image_v1(MEMF *mf)
{
	HostCardInfo_v1	hci;

	if (!mf) return 0;

	if (read_HostCardInfo_v1(mf, 1, &hci)) return 0;

	return 1;
}

int
load_rom_info_v1(MEMF *mf, ROM_info_common *ric, ROM_layout_info_common *rli, int only_chip_layout)
{
	HostCardInfo_v1	hci;
	cardChecksumInfo_v1	cci;
	appInfo_v1	ai[AI_MAX_APPS_V1];
	int	retval = -1;
	int	i;
	char	appname_buf[AI_MAX_APPS_V1][AI_MAX_APPNAME_LEN_V1+1];
	uint8_t	*p8;

	if (!ric) return 1;

	memset((void *)ric, 0, sizeof(*ric));

	ric->ric_rom_version = 1;

	if (read_HostCardInfo_v1(mf, 0, &hci) || read_cardChecksumInfo_v1(mf, &cci)) {
		fprintf(stderr, "ERROR: could not read hci and/or cci!\n");
		goto exit_err;
	}

	if (rli) {
		rli->rli_max_chipsels = ((CCI_MAX_CHIPS_V1 < RIC_MAX_CHIPSELS) ? CCI_MAX_CHIPS_V1 : RIC_MAX_CHIPSELS);
		memset((void *)rli->rli_chipsel_selects, 0xff, sizeof(rli->rli_chipsel_selects));
		rli->rli_num_chipsels = rli->rli_max_chipsels;
		for (i = 0; i < rli->rli_max_chipsels; i++) {
			rli->rli_chipsel_selects[i] = cci.csi_chipSels[i];

			rli->rli_chipsel_baseAddrs[i] = cci.csi_baseAddrs[i];
			rli->rli_chipsel_byteCounts[i] = cci.csi_byteCounts[i];
			rli->rli_chipsel_checksums[i] = cci.csi_checksums[i];

			if ((rli->rli_num_chipsels == rli->rli_max_chipsels) && !rli->rli_chipsel_byteCounts[i]) rli->rli_num_chipsels = i;
		}
	}
	if (only_chip_layout) return 0;


	memset((void *)ai, 0, sizeof(ai));
	for (i = 0; i < hci.ci_numApps; i++) {
		if (read_appInfo_v1(mf, &ai[i])) {
			fprintf(stderr, "ERROR: could not read ai[%d]!\n", i);
			goto exit_err;
		}

		memcpy(appname_buf[i], ai[i].ai_appname, AI_MAX_APPNAME_LEN_V1);
		appname_buf[i][AI_MAX_APPNAME_LEN_V1] = '\0';
		if ((p8 = strchr(appname_buf[0], 0xff))) *p8 = '\0';
	}

	ric->ric_num_apps = hci.ci_numApps;

	for (i = 0; i < hci.ci_numApps; i++) {
		ric->ric_app_names[i] = strdup(appname_buf[i] ?: "");

		ric->ric_appID[i] = ai[i].ai_appID;
		ric->ric_appRev[i] = ai[i].ai_appRev;
		ric->ric_appFlags[i] = ai[i].ai_appFlags;

		ric->ric_p_appEntry[i] = ai[i].ai_entry;
		ric->ric_p_appKeyTrans[i] = ai[i].ai_keytrans;
		ric->ric_p_appIcon[i] = ai[i].ai_icon;
		ric->ric_p_appIrqhandler[i] = ai[i].ai_irqhandler;
		ric->ric_p_appDrawChar[i] = ai[i].ai_drawchar;

		if (!is_known_bad_key_legends_V1(&ai[i])) {
			ric->ric_p_app_red_legend[i] = ai[i].ai_red_legend;
			ric->ric_p_app_green_legend[i] = ai[i].ai_green_legend;
			ric->ric_p_app_yellow_legend[i] = ai[i].ai_yellow_legend;
			ric->ric_p_app_blue_legend[i] = ai[i].ai_blue_legend;
		}
	}

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



int
read_HostCardInfo_v1(MEMF *mf, int quiet, HostCardInfo_v1 *p_out)
{
	if (!p_out) return 1;

	if (mf_seek(mf, 0) != 0) return 1;

	if (mf_read(mf, (void *)p_out, sizeof(*p_out)) != sizeof(*p_out)) return 1;

	if (memcmp(p_out->ci_cardMagic, CARDINFO_MAGIC_ID_V1, CARDINFO_MAGIC_ID_LEN_V1)) {
		if (!quiet) fprintf(stderr, "hci: bad magic!\n");
		return 1;
	}

	endian_fix_LE_val_to_host(&p_out->ci_cardID);
	endian_fix_LE_val_to_host(&p_out->ci_cardRamSize);

	if (!p_out->ci_numApps || (p_out->ci_numApps > AI_MAX_APPS_V1)) {
		if (!quiet) fprintf(stderr, "hci: bad numApps!\n");
		return 1;
	}
	return 0;
}

int
read_cardChecksumInfo_v1(MEMF *mf, cardChecksumInfo_v1 *p_out)
{
	int	i;

	if (!p_out) return 1;

	if (mf_read(mf, (void *)p_out, sizeof(*p_out)) != sizeof(*p_out)) return 1;

	for (i = 0; i < CCI_MAX_CHIPS_V1; i++) {
		endian_fix_LE_val_to_host(&p_out->csi_baseAddrs[i]);
		endian_fix_LE_val_to_host(&p_out->csi_byteCounts[i]);
		endian_fix_LE_val_to_host(&p_out->csi_checksums[i]);
	}
	endian_fix_LE_val_to_host(&p_out->csi_checkfix);

	return 0;
}

int
read_appInfo_v1(MEMF *mf, appInfo_v1 *p_out)
{
	if (!p_out) return 1;

	if (mf_read(mf, (void *)p_out, sizeof(*p_out)) != sizeof(*p_out)) return 1;

	endian_fix_LE_val_to_host(&p_out->ai_entry);
	endian_fix_LE_val_to_host(&p_out->ai_keytrans);
	endian_fix_LE_val_to_host(&p_out->ai_icon);
	endian_fix_LE_val_to_host(&p_out->ai_appID);
	endian_fix_LE_val_to_host(&p_out->ai_appRev);
	endian_fix_LE_val_to_host(&p_out->ai_appFlags);
	endian_fix_LE_val_to_host(&p_out->ai_irqhandler);
	endian_fix_LE_val_to_host(&p_out->ai_drawchar);
	endian_fix_LE_val_to_host(&p_out->ai_red_legend);
	endian_fix_LE_val_to_host(&p_out->ai_green_legend);
	endian_fix_LE_val_to_host(&p_out->ai_yellow_legend);
	endian_fix_LE_val_to_host(&p_out->ai_blue_legend);

	return 0;
}

typedef struct tagV1_BAD_LEGEND_APP_DESC {
	uint16_t	appID;
	uint16_t	appRev;
} V1_BAD_LEGEND_APP_DESC ;

static const V1_BAD_LEGEND_APP_DESC	_known_bad_app_key_legends[] = {
	{ 2006, 0x0100 },	// EMG-2006 v1.0 (wrong legends)
	{ 2008, 0x0100 },	// BTG-2008 v1.0 (junk)
	{ 2009, 0x0100 },	// WNG-2009 v1.0 (junk)
	{ 2013, 0x0100 },	// SLM-2013 v1.0 (junk)
	{ 2014, 0x0100 },	// TMD-2014 v1.0 (junk)
	{ 2015, 0x0100 },	// DBD-2015 v1.0 (junk)
	{ 2023, 0x0100 },	// BDS-2023 v1.0 (junk)
	{ 2023, 0x0101 },	// BDS-2023 v1.1 (junk)
//	{ 2025, 0x0100 },	// BQF-2025 v1.0 (should be French)
	{ 2027, 0x0100 },	// BDF-2027 v1.0 (wrong legends)
	{ 2028, 0x0100 },	// DMF-2028 v1.0 (junk)
};
static const size_t	_sz_known_bad_app_key_legends = (sizeof(_known_bad_app_key_legends) / sizeof(_known_bad_app_key_legends[0]));

static int
is_known_bad_key_legends_V1(appInfo_v1 *ai)
{
	int	i;
	const V1_BAD_LEGEND_APP_DESC	*pinfo;

	if (!ai) return 1;

	for (i = 0, pinfo = &_known_bad_app_key_legends[0]; i < _sz_known_bad_app_key_legends; i++, pinfo++) {
		// NOTE: some apps have 0.0 stored in their header for v1.0, so consider any 0.0 rev a match
		if ((ai->ai_appID == pinfo->appID) && (!ai->ai_appRev || (ai->ai_appRev == pinfo->appRev))) return 1;
	}

	return 0;
}

