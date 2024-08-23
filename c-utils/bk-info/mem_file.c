#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <endian.h>
#include <unistd.h>

#include "mem_file.h"

MEMF *
mf_new(size_t sz_data, int fill_ch)
{
	MEMF	*mf = (MEMF *)0;
	if (!(mf = calloc(1, sizeof(*mf)))) goto exit_err;

	mf->sz_data = sz_data;
	if (!(mf->p_data = (uint8_t *)malloc(sz_data))) goto exit_err;

	if (fill_ch >= 0) memset((void *)mf->p_data, fill_ch, sz_data);

	goto exit_ok;

exit_err:
	if (mf) {
		mf_free(mf);
		mf = (MEMF *)0;
	}
	goto exit_common;

exit_ok:
	// fallthru
exit_common:

	return mf;
}

MEMF *
mf_load(const char *fname)
{
	FILE	*fh = (FILE *)0;
	long	filesize;
	MEMF	*mf = (MEMF *)0;

	if (!fname || !(fh = fopen(fname, "rb"))) goto exit_err;

	fseek(fh, 0L, SEEK_END);
	filesize = ftell(fh);

	fseek(fh, 0L, SEEK_SET);

	if (!(mf = mf_new(filesize, -1))) goto exit_err;

	if (fread((void *)mf->p_data, filesize, 1, fh) != 1) goto exit_err;

	goto exit_ok;

exit_err:
	if (mf) {
		mf_free(mf);
		mf = (MEMF *)0;
	}
	goto exit_common;

exit_ok:
	// fallthru
exit_common:
	if (fh) fclose(fh);

	return mf;
}

int
mf_save(MEMF *mf, const char *fname_out)
{
	FILE	*fh;
	int	ret = 0;

	if (!mf || !mf->p_data || !mf->sz_data || !fname_out) return 1;

	if (!(fh = fopen(fname_out, "wb"))) return 1;

	if (fwrite(mf->p_data, mf->sz_data, 1, fh) != 1) ret = 2;

	fclose(fh);

	if (ret) unlink(fname_out);

	return ret;
}

void
mf_free(MEMF *mf)
{
	if (!mf) return;

	if (mf->p_data) free((void *)mf->p_data);

	free((void *)mf);
}


int
mf_seek(MEMF *mf, loff_t offset)
{
	if (!mf || !mf->p_data || !mf->sz_data || (offset >= mf->sz_data)) return 1;

	mf->cur_offset = offset;

	return 0;
}

loff_t
mf_tell(MEMF *mf)
{
	if (!mf || !mf->p_data || !mf->sz_data) return (loff_t)-1;

	return mf->cur_offset;
}

ssize_t
mf_read(MEMF *mf, uint8_t *p_out, size_t nbytes)
{
	if (!mf || !mf->p_data || !mf->sz_data || (mf->cur_offset >= mf->sz_data)) return -1;

	if ((mf->cur_offset + nbytes) > mf->sz_data) {
		nbytes = (mf->sz_data - mf->cur_offset);
	}

	memcpy((void *)p_out, (void *)&mf->p_data[mf->cur_offset], nbytes);
	mf->cur_offset += nbytes;

	return nbytes;
}

int
mf_getc(MEMF *mf)
{
	uint8_t	ch;

	if (mf_read(mf, &ch, 1) != 1) return -1;

	return (int)ch;
}

ssize_t
mf_filesize(MEMF *mf)
{
	if (!mf || !mf->p_data || !mf->sz_data) return -1;

	return mf->sz_data;
}
