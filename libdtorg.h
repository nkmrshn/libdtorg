#ifndef LIBDTORG_H
#define LIBDTORG_H

typedef struct DTORG_FILELIST_tag {
  char *filename;                     //  Filename
  unsigned char *date_time_original;  //  DateTimeOriginal
  struct DTORG_FILELIST_tag *next;    //  next struct pointer
  struct DTORG_FILELIST_tag *last;    //  last struct pointer
} DTORG_FILELIST;

DTORG_FILELIST *dtorg_read_dir(char *);
DTORG_FILELIST *dtorg_concat_list(DTORG_FILELIST *, DTORG_FILELIST *);
void dtorg_read_list(DTORG_FILELIST *);
void dtorg_dump_list(DTORG_FILELIST *);
void dtorg_free_list(DTORG_FILELIST *);

#endif
