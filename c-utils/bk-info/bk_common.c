#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <endian.h>

#include "bk_common.h"
#include "mem_file.h"
#include "my_endian.h"

int
read_string(MEMF *mf, loff_t offset, char *p_out, int maxlen)
{
	int	i = 0;
	int	ch;

	if (!p_out) return 1;

	if (mf_seek(mf, offset) != 0) return 1;
	while((maxlen < 0) || (i++ < maxlen)) {
		if ((ch = mf_getc(mf)) < 0) return 1;
		ch &= 0xff;
		if (!ch) break;

		*p_out++ = ch;
	}
	*p_out = '\0';

	return 0;
}

int
read_imgImage(MEMF *mf, loff_t offset, imgImage *p_out)
{
	long	filesize = mf_filesize(mf);

	if (!p_out) return 1;

	if (mf_seek(mf, offset) != 0) return 1;

	if (mf_read(mf, (void *)p_out, sizeof(*p_out)) != sizeof(*p_out)) return 1;

	endian_fix_LE_val_to_host(&p_out->i_width);
	endian_fix_LE_val_to_host(&p_out->i_height);
	endian_fix_LE_val_to_host(&p_out->i_rowbytes);
	endian_fix_LE_val_to_host(&p_out->i_bits);

	if (filesize) p_out->i_bits &= (mf_filesize(mf)-1);

	return 0;
}

int
read_host_imgImage(MEMF *mf, loff_t offset, host_imgImage *p_out)
{
	long	filesize = mf_filesize(mf);
	imgImage	l_image;

	if (!p_out) return 1;

	if (read_imgImage(mf, offset, &l_image)) {
		printf("ERR 1\n");
		return 1;
	}

	p_out->i_width = l_image.i_width;
	p_out->i_height = l_image.i_height;
	p_out->i_rowbytes = l_image.i_rowbytes;
	p_out->p_bits = (uint8_t *)0;

	if (l_image.i_bits) {
		size_t	image_bytes = (l_image.i_height * l_image.i_rowbytes);

		if (mf_seek(mf, (loff_t)l_image.i_bits) != 0) return 1;

		if (!(p_out->p_bits = malloc((image_bytes)))) return 1;

		if (mf_read(mf, (void *)p_out->p_bits, image_bytes) != image_bytes) return 1;
	}
	return 0;
}

int
draw_host_imgImage(host_imgImage *p_image, int ntabs_indent)
{
	int	x, y, t;
	uint8_t	*p_row;

	if (!p_image) return 1;

	for (y = 0; y < p_image->i_height; y++) {
		for (t = 0; t < ntabs_indent; t++) {
			printf("\t");
		}
		p_row = (p_image->p_bits + (y * p_image->i_rowbytes));
		for (x = 0; x < p_image->i_width; x++) {
			int	idx = (x / 8);
			uint8_t	mask = (0x80 >> (x & 7));

			if (p_row[idx] & mask) {
				printf("*");
			} else {
				printf(" ");
			}
		}
		printf("\n");
	}
	return 0;
}

int
load_rom_info_common(MEMF *mf, ROM_info_common *ric)
{
	char	red_legend_buf[MAX_KEY_LEGEND_COMMON+1];
	char	green_legend_buf[MAX_KEY_LEGEND_COMMON+1];
	char	yellow_legend_buf[MAX_KEY_LEGEND_COMMON+1];
	char	blue_legend_buf[MAX_KEY_LEGEND_COMMON+1];
	host_imgImage	app_icon;
	int	i, j;
	int	retval = 1;

	if (!mf || !mf->p_data || !mf->sz_data || !ric) return 1;

	for (i = 0; i < ric->ric_num_apps; i++) {
		red_legend_buf[0] = green_legend_buf[0] = yellow_legend_buf[0] = blue_legend_buf[0] = '\0';
		memset((void *)&app_icon, 0, sizeof(app_icon));

		if (ric->ric_p_app_red_legend[i] && read_string(mf, (loff_t)(ric->ric_p_app_red_legend[i] & (mf_filesize(mf)-1)), red_legend_buf, MAX_KEY_LEGEND_COMMON)) goto exit_err;
		if (ric->ric_p_app_green_legend[i] && read_string(mf, (loff_t)(ric->ric_p_app_green_legend[i] & (mf_filesize(mf)-1)), green_legend_buf, MAX_KEY_LEGEND_COMMON)) goto exit_err;
		if (ric->ric_p_app_yellow_legend[i] && read_string(mf, (loff_t)(ric->ric_p_app_yellow_legend[i] & (mf_filesize(mf)-1)), yellow_legend_buf, MAX_KEY_LEGEND_COMMON)) goto exit_err;
		if (ric->ric_p_app_blue_legend[i] && read_string(mf, (loff_t)(ric->ric_p_app_blue_legend[i] & (mf_filesize(mf)-1)), blue_legend_buf, MAX_KEY_LEGEND_COMMON)) goto exit_err;

		if (ric->ric_p_appIcon[i] && read_host_imgImage(mf, (loff_t)(ric->ric_p_appIcon[i] & (mf_filesize(mf)-1)), &app_icon)) {
			fprintf(stderr, "ERROR: could not read icon[%d] (addr=0x%08x, ofs=0x%08x)!\n", i, ric->ric_p_appIcon[i], (ric->ric_p_appIcon[i] & (mf_filesize(mf)-1)));
			goto exit_err;
		}

		ric->ric_red_legends[i] = strdup(red_legend_buf ?: "");
		ric->ric_green_legends[i] = strdup(green_legend_buf ?: "");
		ric->ric_yellow_legends[i] = strdup(yellow_legend_buf ?: "");
		ric->ric_blue_legends[i] = strdup(blue_legend_buf ?: "");

		if ((ric->ric_app_icons[i] = malloc(sizeof(*ric->ric_app_icons[i])))) {
			memcpy((void *)ric->ric_app_icons[i], &app_icon, sizeof(*ric->ric_app_icons[i]));
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

