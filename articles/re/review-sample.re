= Re:VIEW 書き方サンプル

== 節
だんらくだんらく〜〜〜〜
同じ段落

次の段落

=== 項
ソースコードのリスト

//list[main][main()][c]{
int
main(int argc, char **argv)
{
    puts("OK");
    return 0;
}
//}

//listnum[hello][行番号付きコードの例.rb][ruby]{
def hoge
  puts "hello world!"
end
//}

//emlist[][c]{
printf("hello");
//}

//emlistnum[][ruby]{
puts "hello world!"
//}


図はこうだよ

//image[nyancat][nyancat]{
ブロックの中身は無視されるけど画像の説明とか書いておくっぽい
//}

表だよ

//table[envvars][重要な環境変数]{
名前            意味
-------------------------------------------------------------
PATH            コマンドの存在するディレクトリ
TERM            使っている端末の種類。linux・kterm・vt100など
LANG            ユーザのデフォルトロケール。日本語ならja_JP.eucJPやja_JP.utf8
LOGNAME         ユーザのログイン名
TEMP            一時ファイルを置くディレクトリ。/tmpなど
PAGER           manなどで起動するテキスト閲覧プログラム。lessなど
EDITOR          デフォルトエディタ。viやemacsなど
MANPATH         manのソースを置いているディレクトリ
DISPLAY         X Window Systemのデフォルトディスプレイ
//}


文章中で上記の表を指し示すには、インラインで @<table>{envvars} ってかくみたいだよ

==== 段
以下は箇条書きだよ

 * 第1の項目
 ** 第1の項目のネスト
 * 第2の項目
 ** 第2の項目のネスト
 * 第3の項目

 番号付き箇条書きはこうだよ

 1. 第1の条件
 2. 第2の条件
 3. 第3の条件

===== 小段
用語リストがかけるよ

 : Alpha
    DEC の作っていた RISC CPU。
    浮動小数点数演算が速い。

 : POWER
    IBM とモトローラが共同製作した RISC CPU。
    派生として POWER PC がある。

 : SPARC
    Sun が作っている RISC CPU。
    CPU 数を増やすのが得意。

===[column] コラムを書く場所だよ
ここにこんな感じでコラムをかけるよ


===[/column]