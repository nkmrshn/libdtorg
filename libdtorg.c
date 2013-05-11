#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <dirent.h>
#include <regex.h>
#include <stdbool.h>
#include "libdtorg.h"

#define HEADER_BYTES 12

const unsigned char *FILE_FMT = "^[a-zA-Z0-9]{1,8}\\.(JPG|TIF)$";

const unsigned char SOI[] = {0xff, 0xd8};
const unsigned char APP1_MARKER[] = {0xff, 0xe1};
const unsigned char EXIF_IDENT_CODE[] = {0x45, 0x78, 0x69, 0x66, 0x00, 0x00};
const unsigned char TIFF_LITTLE_ENDIAN[] = {0x49, 0x49};
const unsigned char TIFF_BIG_ENDIAN[] = {0x4d, 0x4d};

const unsigned char EXIF_IFD_POINTER_TAG[] = {0x87, 0x69};
const unsigned char DATE_TIME_ORIGINAL_TAG[] = {0x90, 0x03};

unsigned long todecimal(unsigned char *str, size_t s)
{
  int i;
  unsigned long x = 0;

  for(i = 0; i < s; i++) {
    x += str[i] * (unsigned long)pow(256, s - i - 1);
  }

  return x;
}

int strrev(char *str, size_t s)
{
  int i;
  char *tmp;
  
  if((tmp = malloc(sizeof(char) * s)) == NULL)
    return 1;
  
  for(i = 0; i < s; i++) {
    tmp[i] = str[i];
  }
  
  for(i = 1; i <= s; i++) {
    str[i - 1] = tmp[s - i];
  }

  free(tmp);
  tmp = NULL;

  return 0;
}

DTORG_FILELIST *dtorg_read_dir(char *path, int *count)
{
  DTORG_FILELIST *list = NULL, *tmp;
  DIR *dir;
  struct dirent *dp;
  regex_t regst;
  char *actualpath, *delimiter, *pathfmt;
  size_t pathlen, fmtlen;

  *count = 0;

  if((dir = opendir(path)) == NULL)
    return NULL;

  if((actualpath = realpath(path, NULL)) == NULL) {
    closedir(dir);
    return NULL;
  }

  if(regcomp(&regst, FILE_FMT, REG_EXTENDED|REG_NOSUB) != 0) {
    closedir(dir);
    return NULL;
  }

  pathlen = strlen(actualpath);
  delimiter = strrchr(actualpath, '/');

  if(delimiter != NULL && delimiter - actualpath == 0)
    pathfmt = "%s%s";
  else
    pathfmt = "%s/%s";
  
  fmtlen = strlen(pathfmt) - 3;

  for(dp = readdir(dir); dp != NULL; dp = readdir(dir)) {
    if(dp->d_type == DT_REG && !regexec(&regst, dp->d_name, 0, NULL, 0)) {
      char *filename = malloc(sizeof(char) * (pathlen + strlen(dp->d_name) + fmtlen));
      sprintf(filename, pathfmt, actualpath, dp->d_name);
      DTORG_FILELIST *new_list = malloc(sizeof(DTORG_FILELIST));
      
      if(new_list == NULL)
        continue;
      else {
        (*count)++;

        new_list->filename = filename;
        new_list->date_time_original = NULL;
        new_list->next = NULL;
        new_list->last = NULL;

        if(list == NULL) {
          new_list->last = new_list;
          list = new_list;
        } else {
          list->last->next = new_list;
          list->last = new_list;
        }
      }
    }
  }

  regfree(&regst);
  closedir(dir);

  return list;
}

void dtorg_seek(FILE *fp, long offset, int whence, bool *is_valid)
{
  if(is_valid && fseek(fp, offset, whence) != 0)
    is_valid = false;
}

void dtorg_read(void *ptr, size_t s, FILE *fp, bool *is_valid)
{
  if(is_valid && fread(ptr, sizeof(unsigned char), s, fp) != s)
    is_valid = false;
}

void dtorg_reverse(bool is_little_endian, void *ptr, size_t s, bool *is_valid)
{
  if(is_valid && is_little_endian && strrev(ptr, s) != 0)
    is_valid = false;
}

void dtorg_check(const unsigned char *compare, void *ptr, size_t s, bool *is_valid)
{
  if(is_valid && strncmp(compare, ptr, s) != 0)
    is_valid = false;
}

void dtorg_read_list(DTORG_FILELIST *list)
{
  DTORG_FILELIST *tmp = list;
  FILE *fp;
  unsigned char *buffer = malloc(sizeof(unsigned char) * 6);
  bool is_little_endian, is_valid;
  unsigned long i, dtorg_offset, dtorg_count, entry_count;

  if(buffer == NULL)
    return;

  for(tmp = list; tmp != NULL; tmp = tmp->next) {
    //  Initialize
    is_valid = true;  //  assume it is valid
    dtorg_offset = 0;
    dtorg_count = 0;
    entry_count = 0;

    if((fp = fopen(tmp->filename, "rb")) == NULL)
      continue;

    //  SOI
    dtorg_read(buffer, 2, fp, &is_valid);
    dtorg_check(SOI, buffer, 2, &is_valid);

    //  APP1 Marker
    dtorg_read(buffer, 2, fp, &is_valid);
    dtorg_check(APP1_MARKER, buffer, 2, &is_valid);

    //  EXIF Identifier Code
    dtorg_seek(fp, 2, SEEK_CUR, &is_valid);  //  Skip (APP1 Length)
    dtorg_read(buffer, 6, fp, &is_valid);
    dtorg_check(EXIF_IDENT_CODE, buffer, 6, &is_valid);

    //  Byte Order
    dtorg_read(buffer, 2, fp, &is_valid);

    if(is_valid && strncmp(TIFF_LITTLE_ENDIAN, buffer, 2) == 0)
      is_little_endian = true;
    else if(is_valid && strncmp(TIFF_BIG_ENDIAN, buffer, 2) == 0)
      is_little_endian = false;
    else
      is_valid = false;

    //  0th IFD Offset
    dtorg_seek(fp, 2, SEEK_CUR, &is_valid);  //  Skip 0x002A
    dtorg_read(buffer, 4, fp, &is_valid);
    dtorg_reverse(is_little_endian, buffer, 4, &is_valid);

    //  0th IFD
    dtorg_seek(fp, todecimal(buffer, 4) - 8, SEEK_CUR, &is_valid);

    //  0th IFD Entry Count
    dtorg_read(buffer, 2, fp, &is_valid);
    dtorg_reverse(is_little_endian, buffer, 2, &is_valid);

    if(is_valid)
      entry_count = todecimal(buffer, 2);

    //  0th IFD Entry
    for(i = 0; is_valid && i < entry_count; i++) {
      //  Read Tag
      dtorg_read(buffer, 2, fp, &is_valid);
      dtorg_reverse(is_little_endian, buffer, 2, &is_valid);

      //  EXIF IFD Pointer Tag
      if(is_valid && strncmp(EXIF_IFD_POINTER_TAG, buffer, 2) == 0)
        break;

      dtorg_seek(fp, 10, SEEK_CUR, &is_valid); // Skip (Type, Count, Offset)
    }

    //  EXIF IFD Offset
    dtorg_seek(fp, 6, SEEK_CUR, &is_valid);  //  Skip (Type, Count)
    dtorg_read(buffer, 4, fp, &is_valid);
    dtorg_reverse(is_little_endian, buffer, 4, &is_valid);

    //  EXIF IFD
    dtorg_seek(fp, todecimal(buffer, 4) + HEADER_BYTES, SEEK_SET, &is_valid);

    //  EXIF IFD Entry Count
    dtorg_read(buffer, 2, fp, &is_valid);
    dtorg_reverse(is_little_endian, buffer, 2, &is_valid);
    
    if(is_valid)
      entry_count = todecimal(buffer, 2); 

    //  EXIF IFD Entry
    for(i = 0; is_valid && i < entry_count; i++) {
      //  Read Tag
      dtorg_read(buffer, 2, fp, &is_valid);
      dtorg_reverse(is_little_endian, buffer, 2, &is_valid);

      if(is_valid && strncmp(DATE_TIME_ORIGINAL_TAG, buffer, 2) == 0) {
        //  Date Time Orginal Count
        dtorg_seek(fp, 2, SEEK_CUR, &is_valid);  //  Skip (Type)
        dtorg_read(buffer, 4, fp, &is_valid);
        dtorg_reverse(is_little_endian, buffer, 4, &is_valid);

        if(is_valid) {
          dtorg_count = todecimal(buffer, 4);
          tmp->date_time_original = malloc(sizeof(char) * dtorg_count);

          if(tmp->date_time_original == NULL)
            is_valid = false;
        }

        //  Date Time Original Offset
        dtorg_read(buffer, 4, fp, &is_valid);
        dtorg_reverse(is_little_endian, buffer, 4, &is_valid);

        if(is_valid)
          dtorg_offset = todecimal(buffer, 4);

        break;
      } else 
        dtorg_seek(fp, 10, SEEK_CUR, &is_valid);  //  Skip (Type, Count, Offset)
    }

    // Date Time Original Value
    if(is_valid && dtorg_count > 0) {
      dtorg_seek(fp, dtorg_offset + HEADER_BYTES, SEEK_SET, &is_valid);  // offset from beginning
      dtorg_read(tmp->date_time_original, dtorg_count, fp, &is_valid);
    }

    if(fp != NULL)
      fclose(fp);
  }

  free(buffer);
  buffer = NULL;
}

DTORG_FILELIST *dtorg_concat_list(DTORG_FILELIST *dest, DTORG_FILELIST *src)
{
  dest->last->next = src;
  dest->last = src->last;
  src->last = NULL;

  return dest;
}

char ***dtorg_list_array(DTORG_FILELIST *list, int count)
{
  DTORG_FILELIST *tmp;
  char ***array;
  int i;

  array = malloc(count * sizeof(char **));

  for(i = 0, tmp = list; tmp != NULL && i < count; i++, tmp = tmp->next) {
    array[i] = malloc(2 * sizeof(char *));
    array[i][0] = malloc((strlen(tmp->filename) + 1) * sizeof(char));
    strcpy(array[i][0], tmp->filename);

    if(tmp->date_time_original == NULL)
      array[i][1] = NULL;
    else {
      array[i][1] = malloc((strlen(tmp->date_time_original) + 1) * sizeof(char));
      strcpy(array[i][1], tmp->date_time_original);
    }
  }

  return array;
}

int dtorg_asc(const void *p1, const void *p2)
{
  char *dtorg1 = ((char ***)p1)[0][1];
  char *dtorg2 = ((char ***)p2)[0][1];

  if(dtorg1 == NULL && dtorg2 == NULL)
    return 0;
  else if(dtorg1 == NULL)
    return -1;
  else if(dtorg2 == NULL)
    return 1;
  
  return strcmp(dtorg1, dtorg2);
}

int dtorg_desc(const void *p1, const void *p2)
{
  char *dtorg1 = ((char ***)p1)[0][1];
  char *dtorg2 = ((char ***)p2)[0][1];

  if(dtorg1 == NULL && dtorg2 == NULL)
    return 0;
  else if(dtorg1 == NULL)
    return 1;
  else if(dtorg2 == NULL)
    return -1;

  return strcmp(dtorg2, dtorg1);
}

void dtorg_dump_list(DTORG_FILELIST *list)
{
  DTORG_FILELIST *tmp;

  for(tmp = list; tmp != NULL; tmp = tmp->next) {
    printf("%s, %s\n", tmp->filename, tmp->date_time_original);
  }
}

void dtorg_dump_array(char ***array, int count)
{
  int i;

  if(array == NULL)
    return;

  for(i = 0; i < count; i++) {
    printf("%s, %s\n", array[i][0], array[i][1]);
  }
}

void dtorg_free_list(DTORG_FILELIST *list)
{
  DTORG_FILELIST *tmp;

  while(list != NULL) {
    tmp = list;
    list = list->next;
    free(tmp->filename);
    tmp->filename = NULL;

    if(tmp->date_time_original != NULL) {
      free(tmp->date_time_original);
      tmp->date_time_original = NULL;
    }

    free(tmp);
    tmp = NULL;
  }
}

void dtorg_free_array(char ***array, int count)
{
  int i;

  if(array == NULL)
    return;

  for(i = 0; i < count; i++) {
    free(array[i][0]);
    array[i][0] = NULL;

    if(array[i][1] != NULL) {
      free(array[i][1]);
      array[i][1] = NULL;
    }

    free(array[i]);
    array[i] = NULL;
  }

  free(array);
  array = NULL;
}
