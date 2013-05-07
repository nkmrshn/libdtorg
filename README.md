libdtorg
========

指定したディレクトリに保存されているEXIF画像ファイルから、原画増データを生成した日時（DateTimeOriginal）を取得するライブラリです。

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
4. dtorg_dumplist関数で連結リストの内容を標準出力します。
5. 最後に、連結リストをdtorg_free_list関数で解放するのを忘れずに行って下さい。

sample.cをコンパイルする例：

（静的ライブラリ）

    $ gcc -c sample.c
    $ gcc -o sample sample.o libdtorg.a

（共有ライブラリ）

    $ gcc -I./ -L./ sample.c -o sample -ldtorg

連結リスト
----------

連結リストの構造は、libdtorg.hをご覧下さい。

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

備考
----

ライセンスについては、LICENSE.mdをご覧下さい。
