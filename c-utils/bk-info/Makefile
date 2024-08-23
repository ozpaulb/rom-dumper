goodmake: bk_info

ALL_SRC=bk_structs_v1.c bk_structs_v2.c bk_common.c bk_info.c my_endian.c mem_file.c
ALL_HDRS=bk_structs_v1.h bk_structs_v2.h bk_common.h bk_info.h my_endian.h mem_file.h

bk_info: $(ALL_SRC) $(ALL_HDRS)
	gcc -g -o $@ $(ALL_SRC)


clean:
	rm -f bk_info
