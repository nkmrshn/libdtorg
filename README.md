libdtorg
========

指定したディレクトリに保存されているEXIFファイルから、原画像データを生成した日時（DateTimeOriginal）を取得するライブラリです。

ライブラリの作成
----------------

静的ライブラリの生成例：

    $ gcc -c libdtorg.c util.c
    $ ar -r libdtorg.a libdtorg.o util.o

共有ライブラリの生成例：

    $ gcc -shared libdtorg.c -o libdtorg.so

サンプルの概要
--------------

sample.cをコンパイルし、実行すると引数で指定したディレクトリに保存されているEXIFファイルのファイル名と原画像データ生成日時を標準出力します。

詳しくは、sample.cをご覧下さい。

sample.cをコンパイルする例：

（静的ライブラリ）

    $ gcc -c sample.c
    $ gcc -o sample sample.o libdtorg.a

（共有ライブラリ）

    $ gcc -I./ -L./ sample.c -o sample -ldtorg

実行例：

    $ ./sample ~/Pictures/
    /Users/nkmrshn/Pictures/DSCN0001.JPG, 2011:02:23 14:00:03
    /Users/nkmrshn/Pictures/TS3R0001.JPG, 2010:01:01 17:24:02
    /Users/nkmrshn/Pictures/SH340001.JPG, 2008:02:04 06:25:38

    $ ./sample ~/Pictures/ ~/Pictures/another
    /Users/nkmrshn/Pictures/another/DSCN0003.JPG, 2013:05:06 20:32:41
    /Users/nkmrshn/Pictures/another/DSCN0002.JPG, 2013:05:06 20:32:39
    /Users/nkmrshn/Pictures/DSCN0001.JPG, 2011:02:23 14:00:03
    /Users/nkmrshn/Pictures/TS3R0001.JPG, 2010:01:01 17:24:02
    /Users/nkmrshn/Pictures/SH340001.JPG, 2008:02:04 06:25:38

関数一覧
--------

###dtorg_read_dir

* 書式

  DTORG_FILELIST *dtorg_read_dir(char *path, int *count);

* 説明

  pathに指定したディレクトリに保存されているEXIFファイルの連結リストを作成します。ファイル名は、連結リストのfilenameに代入されます。countは、EXIFファイルの個数を代入する整数型ポインタです。

* 戻り値

  連結リストのDTORG_FILELISTのポインタを返します。

---

###dtorg_concat_list

* 書式

  DTORG_FILELIST *dtorg_concat_list(DTORG_FILELIST *dest, DTORG_FILELIST *src);

* 説明

  destの末尾にsrcを連結します。複数のディレクトリにあるEXIFファイルを一つの連結リストで処理したい場合に使います。

* 戻り値

  結果としてできる連結リストdestへのポインタを返します。

---

###dtorg_read_list

* 書式

  void dtorg_read_list(DTORG_FILELIST *list);

* 説明

  DateTimeOriginalをEXIFから取得し、連結リストのdate_time_originalに代入します。取得できなかった場合は、NULLになります。

* 戻り値

  無し。

---

###dtorg_sort

* 書式

  DTORG_FILELIST *dtorg_sort(DTORG_FILELIST *list, int count, enum DTORG_SORT_ORDER order);

* 説明

  結合リストのdate_time_originalでソートします。ソート順は、DTORG_SORT_ASC_ORDER（昇順）もしくはDTORG_SORT_DESC_ORDER（降順）を第3引数で指定します。

* 戻り値

 ソートした連結リストのDTORG_FILELISTのポインタを返します。メモリの割り当てに失敗、あるいはcountが0の場合は、NULLを返します。

---

###dtorg_list_array

* 書式

  char ***dtorg_list_array(DTORG_FILELIST * list, int count);

* 説明

  結合リストを3次元配列化します。countは、EXIFファイルの個数です。

* 戻り値

  3次元配列へのポインタを返します。メモリの割り当てに失敗、あるいはcountが0の場合は、NULLを返します。

---

###dtorg_asc

* 書式
  int dtorg_asc(const void *p1, const void *p2);

* 説明

  3次元配列をqsort関数（stdlib.h）でソートする為の比較関数です。原画像データの生成日時を比較し、昇順になります。

  例）qsort(array, count, sizeof(char ***), dtorg_asc);

* 戻り値

  strcmp関数（string.h）などの比較関数と同じです。

---

###dtorg_desc

* 書式

  int dtorg_desc(const void *p1, const void *p2h);

* 説明

  3次元配列をqsort関数（stdlib.h）でソートする為の比較関数です。原画像データの生成日時を比較し、降順になります。

  例）qsort(array, count, sizeof(char ***), dtorg_desc);

* 戻り値

  strcmp関数（string.h）などの比較関数と同じです。

---

###dtorg_dump_list

* 書式

  void dtorg_dump_list(DTORG_FILELIST *list);

* 説明

  連結リストの内容を標準出力します。

* 戻り値

  無し。

---

###dtorg_dump_array

* 書式

  void dtorg_dump_array(char *** array, int count);

* 説明

  3次元配列の内容を標準出力します。countは、EXIFファイルの個数です。

* 戻り値

  無し。

---

###dtorg_free_list

* 書式

  void dtorg_free_list(DTORG_FILELIST *list);

* 説明

  連結リストをメモリから解放します。

* 戻り値

  無し。

---

###dtorg_free_array

* 書式

  void dtorg_free_array(char ***array, int count);

* 説明

  3次元配列をメモリから解放します。countは、EXIFファイルの個数です。

* 戻り値

  無し。

連結リスト
----------

    typedef struct DTORG_FILELIST_tag {
      char *filename;                   //  ファイル名
      char *date_time_original;         //  原画像データの生成日時
      struct DTORG_FILELIST_tag *next;  //  次の構造体へのポインタ
      struct DTORG_FILELIST_tag *last;  //  最後の構造体へのポインタ
    } DTORG_FILELIST;

備考
----

ライセンスについては、LICENSE.mdをご覧下さい。
