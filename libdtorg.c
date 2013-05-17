#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <dirent.h>
#include <regex.h>
#include <time.h>
#include "util.h"
#include "libdtorg.h"

#define HEADER_BYTES 12

const unsigned char *FILE_FMT = "^[a-zA-Z0-9]{1,8}\\.(JPG|TIF)$";

const unsigned char EXIF_IFD_POINTER_TAG[] = {0x87, 0x69};
const unsigned char DATE_TIME_ORIGINAL_TAG[] = {0x90, 0x03};
  
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
        new_list->since_1970 = 0;
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

void dtorg_read_list(DTORG_FILELIST *list)
{
  DTORG_FILELIST *tmp = list;
  FILE *fp;
  unsigned char *buffer = malloc(sizeof(unsigned char) * 6);
  bool is_little_endian, is_valid;
  unsigned long i, dtorg_offset, dtorg_count, entry_count;
  struct tm date_time;

  if(buffer == NULL)
    return;

  for(tmp = list; tmp != NULL; tmp = tmp->next) {
    //  Initialize
    is_valid = true;  //  assume it is valid
    is_little_endian = NULL;
    dtorg_offset = 0;
    dtorg_count = 0;
    entry_count = 0;

    if((fp = fopen(tmp->filename, "rb")) == NULL)
      continue;

    //  EXIF Header
    read_exif_header(fp, buffer, &is_valid, &is_little_endian);

    //  0th IFD Entry Count
    if(is_valid)
      entry_count = todecimal(buffer, 2);

    //  0th IFD Entry
    for(i = 0; is_valid && i < entry_count; i++) {
      //  Read Tag
      vread(buffer, 2, fp, &is_valid);
      vrev(buffer, 2, &is_valid, &is_little_endian);

      //  EXIF IFD Pointer Tag
      if(is_valid && memcmp(EXIF_IFD_POINTER_TAG, buffer, 2) == 0)
        break;

      vseek(fp, 10, SEEK_CUR, &is_valid); // Skip (Type, Count, Offset)
    }

    //  EXIF IFD Offset
    vseek(fp, 6, SEEK_CUR, &is_valid);  //  Skip (Type, Count)
    vread(buffer, 4, fp, &is_valid);
    vrev(buffer, 4, &is_valid, &is_little_endian);

    //  EXIF IFD
    vseek(fp, todecimal(buffer, 4) + HEADER_BYTES, SEEK_SET, &is_valid);

    //  EXIF IFD Entry Count
    vread(buffer, 2, fp, &is_valid);
    vrev(buffer, 2, &is_valid, &is_little_endian);
    
    if(is_valid)
      entry_count = todecimal(buffer, 2); 

    //  EXIF IFD Entry
    for(i = 0; is_valid && i < entry_count; i++) {
      //  Read Tag
      vread(buffer, 2, fp, &is_valid);
      vrev(buffer, 2, &is_valid, &is_little_endian);

      if(is_valid && memcmp(DATE_TIME_ORIGINAL_TAG, buffer, 2) == 0) {
        //  Date Time Orginal Count
        vseek(fp, 2, SEEK_CUR, &is_valid);  //  Skip (Type)
        vread(buffer, 4, fp, &is_valid);
        vrev(buffer, 4, &is_valid, &is_little_endian);

        if(is_valid) {
          dtorg_count = todecimal(buffer, 4);
          tmp->date_time_original = malloc(sizeof(char) * dtorg_count);

          if(tmp->date_time_original == NULL)
            is_valid = false;
        }

        //  Date Time Original Offset
        vread(buffer, 4, fp, &is_valid);
        vrev(buffer, 4, &is_valid, &is_little_endian);

        if(is_valid)
          dtorg_offset = todecimal(buffer, 4);

        break;
      } else 
        vseek(fp, 10, SEEK_CUR, &is_valid);  //  Skip (Type, Count, Offset)
    }

    // Date Time Original Value
    if(is_valid && dtorg_count > 0) {
      vseek(fp, dtorg_offset + HEADER_BYTES, SEEK_SET, &is_valid);  // offset from beginning
      vread(tmp->date_time_original, dtorg_count, fp, &is_valid);

      if(tmp->date_time_original != NULL) {
        memset(&date_time, 0, sizeof(struct tm));
        sscanf(tmp->date_time_original, "%d:%d:%d %d:%d:%d",
            &date_time.tm_year,
            &date_time.tm_mon,
            &date_time.tm_mday,
            &date_time.tm_hour,
            &date_time.tm_min,
            &date_time.tm_sec);
        date_time.tm_year -= 1900;
        date_time.tm_mon -= 1;
        date_time.tm_isdst = -1;
        tmp->since_1970 = mktime(&date_time);
      }
    }

    if(fp != NULL)
      fclose(fp);
  }

  free(buffer);
  buffer = NULL;
}

int dtorg_sort_asc(const void *p1, const void *p2)
{
  time_t dtorg1 = ((DTORG_FILELIST **)p1)[0]->since_1970;
  time_t dtorg2 = ((DTORG_FILELIST **)p2)[0]->since_1970;

  return difftime(dtorg1, dtorg2);
}

int dtorg_sort_desc(const void *p1, const void *p2)
{
  time_t dtorg1 = ((DTORG_FILELIST **)p1)[0]->since_1970;
  time_t dtorg2 = ((DTORG_FILELIST **)p2)[0]->since_1970;

  return difftime(dtorg2, dtorg1);
}

DTORG_FILELIST *dtorg_sort(DTORG_FILELIST *list, int count, enum DTORG_SORT_ORDER order)
{
  DTORG_FILELIST **array, *tmp;
  int i;

  if(count == 0 || (array = malloc(count * sizeof(DTORG_FILELIST *))) == NULL)
    return NULL;

  for(i = 0, tmp = list; tmp != NULL && i < count; i++, tmp = tmp->next) {
    array[i] = tmp;
  }

  switch(order) {
    case DTORG_SORT_ASC_ORDER:
      qsort(array, count, sizeof(DTORG_FILELIST **), dtorg_sort_asc);
      break;
    case DTORG_SORT_DESC_ORDER:
      qsort(array, count, sizeof(DTORG_FILELIST **), dtorg_sort_desc);
      break;
  }

  for(i = 0; i < count; i++) {
    if(i == 0)
      array[i]->last = array[count - 1];
    else
      array[i]->last = NULL;

    if(i == count - 1)
      array[i]->next = NULL;
    else {
      array[i]->next = array[i + 1];
    }
  }

  tmp = array[0];

  free(array);
  array = NULL;

  return tmp;
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

  if(count == 0 || (array = malloc(count * sizeof(char **))) == NULL)
      return NULL;

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

  if(array == NULL || count == 0)
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

  if(array == NULL || count == 0)
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
