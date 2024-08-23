#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <endian.h>

#include "bk_common.h"
#include "bk_structs_v1.h"
#include "bk_structs_v2.h"
#include "mem_file.h"

static void	_show_appHdr_ptr(const char *p_id, uint32_t addr, uint32_t offs);

int
main(int argc, char *argv[])
{
	const char	*fname_chips[RIC_MAX_CHIPSELS];
	MEMF	*mf_chips[RIC_MAX_CHIPSELS];
	int	mf_num_chips = 0;
	int	retval = 0;
	int	i, j;
	ROM_info_common	ric;
	ROM_layout_info_common	rli;
	size_t	sz_allrom = 0;
	MEMF	*mf_allrom = (MEMF *)0;
	loff_t	offset, top_offset;
	uint16_t	csum;
	uint8_t	*p8;
	int	any_csum_bad = 0;

	if (argc < 2) {
		fprintf(stderr, "Usage: %s filename.bin\n", argv[0]);
		goto exit_err;
	}

	mf_num_chips = 0;
	for (i = 0; i < (argc-1); i++) {
		fname_chips[i] = argv[i+1];

		if (!(mf_chips[i] = mf_load(fname_chips[i]))) {
			fprintf(stderr, "ERROR: could not load '%s'!\n", fname_chips[i]);
			goto exit_err;
		}
		++mf_num_chips;
	}

	if (!mf_num_chips) {
		fprintf(stderr, "ERROR: must specify at least one chip file!\n");
		goto exit_err;
	}

	if (is_bookman_image_v1(mf_chips[0])) {
		if (load_rom_info_v1(mf_chips[0], &ric, &rli, 1)) goto exit_err;
	} else if (is_bookman_image_v2(mf_chips[0])) {
		if (load_rom_info_v2(mf_chips[0], &ric, &rli, 1)) goto exit_err;
	} else {
		fprintf(stderr, "ERROR: unknown ROM file format!\n");
		goto exit_err;
	}

	if (mf_num_chips != rli.rli_num_chipsels) fprintf(stderr, "WARNING: file count (%d) doesn't match chip-select count (%d)!\n", mf_num_chips, rli.rli_num_chipsels);

	// assume all chips are contiguious
	sz_allrom = 0;
	for (i = 0; i < rli.rli_num_chipsels; i++) {
		if ((mf_num_chips > i) && (mf_chips[i]->sz_data > rli.rli_chipsel_byteCounts[i])) {
			fprintf(stderr, "WARNING: file '%s' is larger than chip select region\n", fname_chips[i]);
			rli.rli_chipsel_byteCounts[i] = mf_chips[i]->sz_data;	// FIXME: HACK!
		}
		sz_allrom += rli.rli_chipsel_byteCounts[i];
	}
	if (!sz_allrom) {
		fprintf(stderr, "ERROR: could not determine total ROM memory region size!\n");
		goto exit_err;
	}
sz_allrom = (16*1048576);

	if (!(mf_allrom = mf_new(sz_allrom, 0xff))) {
		fprintf(stderr, "ERROR: could not allocate 0x%08lx bytes for ROM memory region!\n", sz_allrom);
		goto exit_err;
	}

	offset = 0;
	for (i = 0; i < mf_num_chips; i++) {

		switch(ric.ric_rom_version) {
		case 1:	// ROM chips start at 0 and grow up
			memcpy((void *)&mf_allrom->p_data[offset], (void *)mf_chips[i]->p_data, mf_chips[i]->sz_data);
			break;
		case 2:	// ROM chips start at top of memory and grow down
			top_offset = mf_allrom->sz_data - offset - mf_chips[i]->sz_data;
			memcpy((void *)&mf_allrom->p_data[top_offset], (void *)mf_chips[i]->p_data, mf_chips[i]->sz_data);
			break;
		default:
			break;
		}
		offset += rli.rli_chipsel_byteCounts[i];
	}

	csum = 0;
	memset((void *)rli.rli_chipsel_checksums_good, 0, sizeof(rli.rli_chipsel_checksums_good));
	rli.rli_chipsel_all_checksums_good = 0;

	offset = 0;
	any_csum_bad = 0;
	for (i = 0; i < rli.rli_num_chipsels; i++) {
		if ((rli.rli_chipsel_baseAddrs[i] >= mf_filesize(mf_allrom)) ||
			((rli.rli_chipsel_baseAddrs[i] + rli.rli_chipsel_byteCounts[i]) > mf_filesize(mf_allrom))) {
			any_csum_bad = 1;
			continue;
		}
		switch(ric.ric_rom_version) {
		case 1:	// ROM chips start at 0 and grow up
			p8 = (uint8_t *)&mf_allrom->p_data[offset];
			break;
		case 2:	// ROM chips start at top of memory and grow down
			top_offset = mf_allrom->sz_data - offset - rli.rli_chipsel_byteCounts[i];
			p8 = (uint8_t *)&mf_allrom->p_data[top_offset];
			break;
		default:
			break;
		}
		csum = 0;
		for (j = 0; j < rli.rli_chipsel_byteCounts[i]; j++) {
			csum += *p8++;
		}

		if (csum == rli.rli_chipsel_checksums[i]) {
			rli.rli_chipsel_checksums_good[i] = 1;
		} else {
			any_csum_bad = 1;
		}
//		printf("csum[%d]: %04x, expect: %04x any_csum_bad=%d %s\n", i, csum, rli.rli_chipsel_checksums[i], any_csum_bad, (rli.rli_chipsel_checksums_good[i] ? "GOOD" : "BAD"));

		offset += rli.rli_chipsel_byteCounts[i];
	}
	if (!any_csum_bad) rli.rli_chipsel_all_checksums_good = 1;
mf_save(mf_allrom, "out_allrom.bin");
	switch(ric.ric_rom_version) {
	case 1:
		if (load_rom_info_v1(mf_allrom, &ric, NULL, 0)) goto exit_err;
		break;
	case 2:
		if (load_rom_info_v2(mf_allrom, &ric, NULL, 0)) goto exit_err;
		break;
	default:
		fprintf(stderr, "ERROR: unknown ROM file format version!\n");
		goto exit_err;
	}


	if (load_rom_info_common(mf_allrom, &ric)) goto exit_err;

	printf("File: '%s' (BOOKMAN image version %d, csum(s) %s)\n", fname_chips[0], ric.ric_rom_version, (rli.rli_chipsel_all_checksums_good ? "good" : "bad"));
	for (i = 0; i < ric.ric_num_apps; i++) {
		printf("App #%d\n", i);
		printf("\tName: '%s'\n", ric.ric_app_names[i]);
		printf("\tappID: %d\n", ric.ric_appID[i]);

		printf("\tappRev: %d.%d\n", ((ric.ric_appRev[i] >> 8) & 0xff), (ric.ric_appRev[i] & 0xff));
		if (ric.ric_appFlags[i]) printf("\tappFlags: 0x%04x\n", ric.ric_appFlags[i]);

		_show_appHdr_ptr("p_appEntry", ric.ric_p_appEntry[i], (ric.ric_p_appEntry[i] & (mf_filesize(mf_allrom)-1)));
		_show_appHdr_ptr("p_appKeyTrans", ric.ric_p_appKeyTrans[i], (ric.ric_p_appKeyTrans[i] & (mf_filesize(mf_allrom)-1)));
		_show_appHdr_ptr("p_appIconSize", ric.ric_p_appIconSize[i], (ric.ric_p_appIconSize[i] & (mf_filesize(mf_allrom)-1)));
		_show_appHdr_ptr("p_appIconDraw", ric.ric_p_appIconDraw[i], (ric.ric_p_appIconDraw[i] & (mf_filesize(mf_allrom)-1)));
		_show_appHdr_ptr("p_appIcon", ric.ric_p_appIcon[i], (ric.ric_p_appIcon[i] & (mf_filesize(mf_allrom)-1)));
		_show_appHdr_ptr("p_biosBookInfo", ric.ric_p_biosBookInfo[i], (ric.ric_p_biosBookInfo[i] & (mf_filesize(mf_allrom)-1)));
		_show_appHdr_ptr("p_appIrqhandler", ric.ric_p_appIrqhandler[i], (ric.ric_p_appIrqhandler[i] & (mf_filesize(mf_allrom)-1)));
		_show_appHdr_ptr("p_appDrawChar", ric.ric_p_appDrawChar[i], (ric.ric_p_appDrawChar[i] & (mf_filesize(mf_allrom)-1)));
		_show_appHdr_ptr("p_appRedLegend", ric.ric_p_app_red_legend[i], (ric.ric_p_app_red_legend[i] & (mf_filesize(mf_allrom)-1)));
		_show_appHdr_ptr("p_appGreenLegend", ric.ric_p_app_green_legend[i], (ric.ric_p_app_green_legend[i] & (mf_filesize(mf_allrom)-1)));
		_show_appHdr_ptr("p_appYellowLegend", ric.ric_p_app_yellow_legend[i], (ric.ric_p_app_yellow_legend[i] & (mf_filesize(mf_allrom)-1)));
		_show_appHdr_ptr("p_appBlueLegend", ric.ric_p_app_blue_legend[i], (ric.ric_p_app_blue_legend[i] & (mf_filesize(mf_allrom)-1)));

		printf("\tRED: [%s]\n", ric.ric_red_legends[i]);
		printf("\tGREEN: [%s]\n", ric.ric_green_legends[i]);
		printf("\tYELLOW: [%s]\n", ric.ric_yellow_legends[i]);
		printf("\tBLUE: [%s]\n", ric.ric_blue_legends[i]);

		if (ric.ric_app_icons[i]) {
			draw_host_imgImage(ric.ric_app_icons[i], 1);
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
	for (i = 0; i < mf_num_chips; i++) {
		if (mf_chips[i]) mf_free(mf_chips[i]);
	}
	if (mf_allrom) mf_free(mf_allrom);

	return retval;
}

static void
_show_appHdr_ptr(const char *p_id, uint32_t addr, uint32_t offs)
{
	if (!addr || (addr == 0xffffffff)) return;

	printf("\t%s: 0x%08x", p_id, addr);
	if (offs != addr) {
		printf(" (offset: 0x%08x)", offs);
	}
	printf("\n");

}
