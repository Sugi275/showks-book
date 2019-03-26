= 触れる・作れる・持って帰れる 参加型ショーケース[showKs]

== これがshowKsだ！

フォームに入力したら

//image[01/pre1][フォーム]{
//}

あなた専用のキャンバスが出来ていて

//image[01/pre2][フォーム]{
//}

GitHubリポジトリやCIパイプラインやCDパイプラインが出来ている。

//image[01/pre3][フォーム]{
//}
//image[01/pre4][フォーム]{
//}
//image[01/pre5][フォーム]{
//}

そしてポータルでみんなの絵が見られる。

//image[01/pre6][フォーム]{
//}

これが、参加者の視点として見られるshowKsです。これだけでは、単にお絵描きアプリをデプロイするだけの仕組みに思えてしまうかもしれませんね。では、参加者として可能なことをもう少し深掘りしてみましょうか。

まず、こちらがshowKsの全体アーキテクチャです。

//image[01/pre7][フォーム]{
//}

== Cloud Nativeのショーケース作ろう -企画のきっかけ

showKsは、[K]ubernetesを用いたCI/CDや開発環境を実際に試せるショーケースとして命名されました。
最近ではKubernetes

[column] <解説>そもそもCloud Nativeってなんだっけ
Cloud Nativeはバズワードとして語られているように見えて、実はLinux FoundationのサブプロジェクトのCloud Native Computing Foundation（CNCF）のTOCが定義を公開しています（https://github.com/cncf/toc/blob/master/DEFINITION.md）。
この定義を簡潔に表現するとCloud Nativeな技術を利用することで、
・疎結合なシステム
・復元力がある
・管理しやすい
・可観測である
・堅牢な自動化により、頻繁かつ期待通りに最小限の労力で大きな変更が可能
といった特徴を持ったシステムをオープンなテクノロジーを利用してスケーラブルに実現できるようになります。
このCloud Native自体はKubernetes・Docker・マイクロサービスなどが必須なわけではありません。
しかし、こういった技術を利用することによりCloud Nativeを実現する近道となります。
CloudからCloud Nativeへ移行するにあたっては、Trail Map（https://github.com/cncf/trailmap）についても参考にしてみてください。
なお、showKsでもこのTrail Mapにある「Containerization」「CI/CD」「Orchestration & Application Definition」「Observaility & Analysis」といった順に進んでいきCloud Nativeを実現していきました。
===[/column]

== いきなり暗礁に

== 救世主登場

== 活動時間は22:00-28:00

== あー

== 全てお持ち帰りできるぜ

== ツールドリブン万歳



クラウドネイティブな開発を体験してもらおう
何を見せたい？ ツールをきめよう
Spinnakerは
ツールドリブンな企画だけど実際には・・
CDどうしよう？⇒ Spinnakerでやろうと思えばやれる・・・
GitOpsっていう考え方があってね
CI必要だよね
