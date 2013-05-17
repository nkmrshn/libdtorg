#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "libdtorg.h"

int main(int argc, char *argv[])
{
  DTORG_FILELIST *list, *tmp;   //  連結リスト
  char ***array;                //  三次元配列
  int i;
  int count;                    //  連結リストの個数
  int total = 0;

  //  EXIFファイルの連結リストを取得
  if(argc > 1) {
    //  実行時、複数のディレクトリが引数に指定してあった場合
    for(i = 1; i < argc; i++) {
      //  連結リストの取得
      if((tmp = dtorg_read_dir(argv[i], &count)) == NULL)
          continue;

      total += count;

      if(list == NULL)
        list = tmp;
      else
        dtorg_concat_list(list, tmp);   //  連結リストを結合
    }
  } else  //  引数を指定してなかった場合
    list = dtorg_read_dir(".", &total);   //  カレントディレクトリ

  if(list != NULL) {
    //  撮影日時の取得
    dtorg_read_list(list);

    //  連結リストのソート
    //list = dtorg_sort(list, total, DTORG_SORT_ASC_ORDER);
    list = dtorg_sort(list, total, DTORG_SORT_DESC_ORDER);

    //  連結リストの内容を表示
    dtorg_dump_list(list);

    //  連結リストの解放
    dtorg_free_list(list);
  }

  return 0;
}
