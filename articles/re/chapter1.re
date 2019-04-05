= 触れる・作れる・持って帰れる 参加型ショーケース[showKs]

@<icon>{01/opening1}

@<icon>{01/opening2}

@<icon>{01/opening3}

これが、参加者の視点として見られるshowKsです。これだけでは、単にお絵描きアプリをデプロイするだけの仕組みに思えてしまうかもしれませんね。でも、この仕組みの裏には、いわゆる『クラウドネイティブ』な要素を沢山盛り込んであるんです。 その仕組みを説明していく前に、そもそもなんで僕たちがこんな企画を始めたのかをお話させてください。

== Cloud Nativeのショーケース作ろう -企画のきっかけ

showKsは、[K]ubernetesを用いたCI/CDや開発環境を実際に試せるショーケースとして命名されました。

JKD v18.12が開催されたのは12月ですが、実行委員会が組織されたのはそれより半年以上前。なんせ数百人を集めるカンファレンスですから、準備には結構時間がかかるのです。

そうして開催された6月某日の実行委員会。そこで、JKDのオーガナイザーである鈴木さん(@szkn27) から『JKDの主催者側で何か企画をやってみないか』という話が持ちかけられました。 クラウドやコンテナ技術で名が知られている人も運営メンバーとして多く参加しているJKD。そのメンバーで何か企画をやれる、しかもかかる費用は主催者持ち。 こんな楽しいことやれそうなチャンス、なかなかありません。 ということで、テンションの上がった我々は様々なアイディアを出してみました。

その中でも最も有望そうだったのが、『クラウドネイティブ技術のショーケース』 という案でした。 この案は、ネットワーク技術のカンファレンス 『Interop Tokyo@<fn>{interop}』 で行われているプロジェクト、『ShowNet』の影響を強く受けています。Interopというイベント名は、相互運用性(Interoperability)から来ているのですが、そのイベント名を体現するかのごとく、最新の機器と最新の技法で、会場内のネットワークを組み上げるという企画がShowNetなのです。25年もの歴史を持つ企画なので、ご存じの方も多いかも知れませんね。 

現在、さまざまなクラウドベンダーがKubernetes(以下k8s)のマネジメントサービスを提供しています。オンプレにk8sを構築するソリューションも多く登場しています。 こういった各サービス・ソフトウェア間を跨がるソフトウェアを作り、相互運用性を確認するような企画はどうかと。

企画の方向性はこれで決まったので、次は企画名を決めたくなってきました。イベントまでは半年もあるので、急いで企画名を決める必要は無かったのですが、仮でもいいから名前があったほうが、テンションがあがりますよね。『名前は、ついていることが大切だ』(大河内首相 - シン・ゴジラ)

『ショーケース･･･Kubernetes･･･クラウドネイティブ』 『show? showk8s? ShowKon?』 『なんか語呂悪くない？』　『じゃあshowK』 『えー、なんかショーンKっぽくて嫌だ』　『じゃあshow"Kace" とか showKsってどう？ショーケースって発音にできるし』　『それだ！』

本当はもう少しいろんなやり取りがあったのですが、おおよそこんなノリで名前が決まりました。こうして、クラウドネイティブのショーケース『showKs』企画がスタートしたのです。

//footnote[interop][https://www.interop.jp/]

===[column] <解説>そもそもCloud Nativeってなんだっけ

Cloud Nativeはバズワードとして語られているように見えて、実はLinux FoundationのサブプロジェクトのCloud Native Computing Foundation（CNCF）のTOCが定義を公開しています@<fn>{cncf-toc}。

この定義を簡潔に表現するとCloud Nativeな技術を利用することで、

 * 疎結合なシステム
 * 復元力がある
 * 管理しやすい
 * 可観測である
 * 堅牢な自動化により、頻繁かつ期待通りに最小限の労力で大きな変更が可能

といった特徴を持ったシステムをオープンなテクノロジーを利用してスケーラブルに実現できるようになります。
このCloud Native自体はKubernetes・Docker・マイクロサービスなどが必須なわけではありません。
しかし、こういった技術を利用することによりCloud Nativeを実現する近道となります。
CloudからCloud Nativeへ移行するにあたっては、Trail Map@<fn>{trailmap}についても参考にしてみてください。
なお、showKsでもこのTrail Mapにある「Containerization」「CI/CD」「Orchestration & Application Definition」「Observaility & Analysis」といった順に進んでいきCloud Nativeを実現していきました。

//footnote[cncf-toc][https://github.com/cncf/toc/blob/master/DEFINITION.md]
//footnote[trailmap][https://github.com/cncf/trailmap]


===[/column]

== いきなり暗礁に

勢いよく始まったかに見えたshowKsですが、実際にはいきなり暗礁に乗り上げてしまいました。企画の具体的な内容を考えているうちに、僕らは気づいてしまったのです。『Kubernetesの相互運用性確認って、ぶっちゃけあまりすることないじゃん』と。

前述したように、僕らが参考にしたShowNetはネットワーク機器の相互運用性を確認する企画でした。ネットワーク機器は、あらかじめ定められているネットワークプロトコルを扱えるよう各ベンダーがそれぞれの実装を行っています。規格は定められているもの、実装はベンダーに委ねれているため、規格を満たしていない・規格の解釈違い・実装のバグなどが起きえます。そのため、同じ規格に対応しているはずなのに上手く繋がらないという事象が頻繁に起こります。これらの問題を確認する場として、ShowNetは機能しているのです。

一方、僕らがやろうとしていたshowKsは、どのベンダーのサービスであっても、基本的に同一のコードベースを利用しているはずなのです。ベンダー独自の拡張についても、KubernetesはCustom Resourceという仕組みで拡張可能のため、Kubernetes本体をフォークする必要性があまり無いのですね。強いて言えばバージョン違いくらいのものでしょうか。

なので、あるManaged Kubernetesで動いているアプリケーションは、そのまま他のKubernetesに移行できてしまいます。Persistent VolumeやLoadBalancerなどの差分はありますが、それらの差分を修正するのはそれほど難しいことではありません。

つまり、僕らの企画は最初からあまり需要がないものだったのです。なんてこった･･･

== 救世主登場

そうして気がついたら、8月も終わろうとしていました。企画が始まったのが6月後半。1ヶ月半の間で決まったのは企画名だけという有様でした。

このまま企画ごと流れてしまうのかな、という雰囲気が漂い始めたそのとき、救世主が登場したのです。その名も、青山真也(@amsy810)。名著『Kubernetes完全ガイド』の著者でもある彼が提案したのは、以下のようなアイディアでした。

@<br>{}@<b>{CI/CD体験コーナー}
 * パブリックリポジトリを用意しておいて、そこに参加者がPR・マージすると各種ツールが走ってデプロイされる
 * 誰でも書ける簡単なGo appを想定(hello, hogehogeを書き換えるレベル)
 * そのCI/CD環境を実際に触れる(SpinnakerのRead Onlyアカウントを払い出しておくなど)
 * 結果はコーナーに置いてあるモニターで逐一表示させておく
 * パネルにアーキテクチャ図を書いておく
 * 設定例一式のお持ち帰りができる

どうでしょう。ふわっとした当初案に比べて、具体的に何をやれば良いのかイメージがしやすい案に思えますね。体験できるだけでなく、設定例一式のお持ち帰りができるという点もユニークです。ハードウェアではなく、オープンソースなソフトウェアで完結できるテクノロジーならではの企画じゃないでしょうか。

企画において『関係者全員がイメージしやすい』ことは非常に重要ですね。具体的な案が出たことで停滞していた企画は一気に活性化し、新たなスタートを切ることとなったのです。

== 参加型アプリを作ろう

さて、無事リブートしたshowKs。オンライン・オフラインで何度か打ち合わせを行った結果、当初よりもアプリケーション部分を膨らませたいという気もちになって来ました。メンバーのひとりである@jyoshiseから、以下のようなアイディアが提案されたのです。

 * どうせなら、参加した人が足跡を残せるようなやつが良いんでは無いか？
 * 『みんなが参加してくれた結果、最終的にこんなんになりました！』と俯瞰して成果を見られるようなアプリに
 ** Redditの企画"Place"みたいなやつ@<fn>{place}

このアイディアを叩き台として、@jyoshise, @kojiha, @tsukamanらにより練るれたアイディアが『お絵描きアプリ』でした。

 * 参加者はshowKsに登録すると、専用のキャンバスが1つ与えられる
 * キャンバスには参加者固有のSNSアカウントやメッセージなどを埋め込める
 * キャンバスをブラウザで開いたユーザは自由にペンで書き込むことができる
 * 全てのキャンバスを一カ所に集め、俯瞰して見ることのできるポータルを用意する

実装方法はいろいろありそうですが、参加者ごとのキャンバスとポータルをそれぞれ別のアプリとして稼働させ、連動するようにすればマイクロサービスアーキテクチャのように振る舞うことができそうです。 Kubernetesを利用するという方向性にもマッチしているんじゃないでしょうか。 

//indepimage[01/discussion][ディスカッション風景。この時点でおおよそのアプリの方向性が決まっていた][scale=0.5]
//footnote[place][http://www.cbc-net.com/posts/2017/04/11/reddit-place/]

== コンセプトの決定

こうして、全体のコンセプトが決まりました。

 * クラウドネイティブ開発の体験が出来る
 ** アプリごとのリポジトリ
 ** CI/CDパイプライン
 ** PullRequestベースの開発
 * 参加者全員で@<b>{共同作業感}を出す
 ** マイクロサービスっぽさ
 ** ポータルによる俯瞰
 ** 他者キャンバスへのお絵描き追記
 * @<b>{全てお持ち帰り可能}
 ** アプリケーションコード
 ** マニフェスト
 ** ドキュメント
 
青山案にあった『体験できる』『持ち帰りができる』というキーコンセプトを維持しつつ、より具体的な『クラウドネイティブ開発』の要素を加える形になりました。なかなか楽しい企画になりそうです。
