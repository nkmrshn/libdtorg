libdtorg
========

指定したディレクトリに保存されているEXIF画像ファイルから、原画像データを生成した日時（DateTimeOriginal）を取得するライブラリです。

ライブラリの作成
----------------

静的ライブラリの生成例：

    $ gcc -c libdtorg.c
    $ ar -r libdtorg.a libdtorg.o

共有ライブラリの生成例：

    $ gcc -shared libdtorg.c -o libdtorg.so

サンプルの概要
--------------

詳しくは、sample.cをご覧下さい。

1. libdtorg.hをincludeしてください。
2. dtorg_read_dir関数の引数にディレクトリを渡すと、EXIF画像ファイルの連結リストを作成します。
3. 続いて、dtorg_read_list関数にこの連結リストを渡すと、DateTimeOriginalをEXIFから取得します。
4. dtorg_dump_list関数で連結リストの内容を標準出力します。
5. 最後に、連結リストをdtorg_free_list関数で解放するのを忘れずに行って下さい。

sample.cをコンパイルする例：

（静的ライブラリ）

    $ gcc -c sample.c
    $ gcc -o sample sample.o libdtorg.a

（共有ライブラリ）

    $ gcc -I./ -L./ sample.c -o sample -ldtorg

関数一覧
--------

###dtorg_read_dir

* 書式

  DTORG_FILELIST *dtorg_read_dir(char *path);

* 説明

  pathに指定したディレクトリに保存されているEXIF画像ファイルの連結リストを作成します。ファイル名は、連結リストのfilenameに代入されます。

* 戻り値

  連結リストのDTORG_FILELISTのポインタを返します。

###dtorg_concat_list

* 書式

  DTORG_FILELIST *dtorg_concat_list(DTORG_FILELIST *dest, DTORG_FILELIST *src);

* 説明

  destの末尾にsrcを連結します。

* 戻り値

  結果としてできる連結リストdestへのポインタを返します。

###dtorg_read_list

* 書式

  void dtorg_read_list(DTORG_FILELIST *list);

* 説明

  DateTimeOriginalをEXIFから取得し、連結リストのdate_time_originalに代入します。取得できなかった場合は、NULLになります。

* 戻り値

  無し。

###dtorg_dump_list

* 書式

  void dtorg_dump_list(DTORG_FILELIST *list);

* 説明

  連結リストの内容を標準出力します。

* 戻り値

  無し。

###dtorg_free_list

* 書式

  void dtorg_free_list(DTORG_FILELIST *list);

* 説明

  連結リストをメモリから解放します。

* 戻り値

  無し。

連結リスト
----------

    typedef struct DTORG_FILELIST_tag {
      char *filename;                     //  ファイル名
      unsigned char *date_time_original;  //  原画像データの生成日時
      struct DTORG_FILELIST_tag *next;    //  次の構造体へのポインタ
      struct DTORG_FILELIST_tag *last;    //  最後の構造体へのポインタ
    } DTORG_FILELIST;

備考
----

ライセンスについては、LICENSE.mdをご覧下さい。
