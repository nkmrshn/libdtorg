#ifndef LIBDTORG_H
#define LIBDTORG_H

typedef struct DTORG_FILELIST_tag {
  //  Filename
  char *filename;
  //  Datetime Original
  unsigned char *date_time_original;
  //  next FILELIST pointer
  struct DTORG_FILELIST_tag *next;
  //  last FILELIST pointer
  struct DTORG_FILELIST_tag *last;
} DTORG_FILELIST;

DTORG_FILELIST *dtorg_read_dir(char *);
void dtorg_read_list(DTORG_FILELIST *);
void dtorg_dump_list(DTORG_FILELIST *);
void dtorg_free_list(DTORG_FILELIST *);

#endif
