#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "libdtorg.h"

int main(int argc, char *argv[])
{
  DTORG_FILELIST *list;   //  連結リスト
  char *path;             //  ディレクトリ

  //  ディレクトリを取得
  if(argc > 1) {
    //  引数
    path = malloc(sizeof(char) * (strlen(argv[1]) + 1));
    strcpy(path, argv[1]);
  } else {
    //  カレントディレクトリ
    path = malloc(sizeof(char) * 2);
    strcpy(path, ".");
  }
 
  //  EXIF画像ファイルの連結リストを取得
  if((list = dtorg_read_dir(path)) != NULL) {
    //  撮影日時の取得
    dtorg_read_list(list);

    //  連結リストの内容を表示
    dtorg_dump_list(list);
  }

  //  連結リストの解法
  dtorg_free_list(list);

  //  ディレクトリの解法
  free(path);

  return 0;
}
