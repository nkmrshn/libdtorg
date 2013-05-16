#ifndef LIBDTORG_H
#define LIBDTORG_H

typedef struct DTORG_FILELIST_tag {
  char *filename;                   //  Filename
  char *date_time_original;         //  DateTimeOriginal
  struct DTORG_FILELIST_tag *next;  //  next struct pointer
  struct DTORG_FILELIST_tag *last;  //  last struct pointer
} DTORG_FILELIST;

enum DTORG_SORT_ORDER {
  DTORG_SORT_ASC_ORDER,
  DTORG_SORT_DESC_ORDER
};

DTORG_FILELIST *dtorg_read_dir(char *, int *);
DTORG_FILELIST *dtorg_concat_list(DTORG_FILELIST *, DTORG_FILELIST *);
void dtorg_read_list(DTORG_FILELIST *);
DTORG_FILELIST *dtorg_sort(DTORG_FILELIST *, int, enum DTORG_SORT_ORDER);
char ***dtorg_list_array(DTORG_FILELIST *, int);
int dtorg_asc(const void *, const void *);
int dtorg_desc(const void *, const void *);
void dtorg_dump_list(DTORG_FILELIST *);
void dtorg_dump_array(char ***, int);
void dtorg_free_list(DTORG_FILELIST *);
void dtorg_free_array(char ***, int);

#endif
