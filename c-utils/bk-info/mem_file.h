#ifndef MEM_FILE_H
#define	MEM_FILE_H

typedef struct tagMEMF {
	uint8_t	*p_data;
	size_t	sz_data;
	loff_t	cur_offset;
} MEMF;

extern	MEMF	*mf_new(size_t sz_data, int fill_ch);
extern	MEMF	*mf_load(const char *fname);
extern	int	mf_save(MEMF *mf, const char *fname_out);
extern	void	mf_free(MEMF *mf);

extern	int	mf_seek(MEMF *mf, loff_t offset);
extern	loff_t	mf_tell(MEMF *mf);

extern	ssize_t	mf_read(MEMF *mf, uint8_t *p_out, size_t nbytes);
extern	int	mf_getc(MEMF *mf);

extern	ssize_t	mf_filesize(MEMF *mf);

#endif // MEM_FILE_H
