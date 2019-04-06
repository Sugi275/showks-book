= これからも続くshowKs

//lead{
  JKD v1812におけるshowKsは、当初の目的を果たすという意味では十分な成果を残せたと信じています。
しかし、本当はやりたかっけども、限られた時間の中で取捨選択をしなければいけないことも沢山ありました。
走りながら作り上げていくスタイルだったこともあり、次から次へとやりたいことが出てきたのですが、「次に」持ち越す決断をせざるを得ませんでした。

しかし、Japan Container DaysがCloud Native Daysとしてこれからも続いていくように、showKsプロジェクトもまだまだ進化を目指しています。
なにせJKD本番当日でさえずっと機能改善に取り組んだくらいなんですから、ここで終わりにしようなんて誰も考えていませんでした。
showKsそのものがContinuousなProjectとして、改良・進化を目指すものとなったのです。

この第4章では、そんな「次」に何をしようと企んでいるか、ご紹介したいと思います。
//}

== CustomResourceDefinitionの活用

Kubernetesの特徴の一つとしてよく取り上げられるのが拡張性です。PodやDeploymentのような標準APIだけではなく、自らCustom Resourceを定義し、Kubernetes上で扱うことが可能なんです。カスタムリソースを定義する機能のことをCustomResourceDefinition（CRD）といいます。

今後、よりKubernetesネイティブな開発をしていくにあたっては、このCRDをいかに活用していくかがポイントではないかと考えています。

たとえば、第2章で紹介したshowks-formは、申し込みに応じてGitHubリポジトリをプロビジョニングし、ConcourseとSpinnakerのPipelineを作成するコンポーネントでしたね。作成のみでなく削除機能も実装してあるのですが、いずれの機能もユーザー名をキーに、各サービスのAPIを叩きに行くという実装になっています。どういうユーザーが登録しているかという状態の管理は、showks-formのデータベースに依存してることになります。

例えばここにCRDを活用すると、リソースの状態管理はKubernetes側に委ねることができるようになります。"ShowksUser"リソースを作成すると、それが自動で"GitHub", "Concourse", "Spinnaker"というリソースを作成し、それらリソースは各サービスのAPIを叩きに行って実体を生成すると。削除するときは、Kubernetesでこれらのリソースを削除すれば実体も消える･･･と。この仕組みならば、ユーザーのプロビジョニングも第2章で解説したGitOpsの仕組みに載せることができそうですね。

こういった実装が本当に工数に見合うものになるかどうかはまだ分かりません。そこまでしてCRD化進めない方が効率がいいという可能性も十分にあり得ます。そのあたりの知見を貯めるためにも、次の回では是非挑戦してみたいと考えています。

===[column] 【コラム】CustomResourceDefinitionとOperator

CustomResourceDefinitionの目的は、手動でやっていたオペレーションやミドルウェアの管理をプログラム化して拡張することです。
このプログラムのことをCustom ControllerやOperatorと呼んだりしています。
例えばMySQLの管理やスケールを自動化してくれるMySQL Operatorを使ってMySQLを構築した場合には、Database as a Service on Kubernetesのように捉えることも可能ですね。
こういった背景から、KubernetesはクラウドのOSになると言われたり、小さなGCPやAWSのように見えるところもあります。

===[/column]

== Service MeshとIstio

showKsを作っていく中で、いろんなところで「マイクロサービス間の通信制御」「外部からのアクセス制御」といった問題に遭遇しました。

例えばcanvasアプリがデプロイされた際のportalアプリへの反映。今回はKubernetesのサービスディスカバリを利用してaggregatorがcanvasを監視する仕組みをとりましたが、Service Meshがあればもっとスマートな実装ができたのではないか。

例えばカナリアリリース。Productionクラスタには、本番稼働中のアプリケーションに影響を与えかねない不確かなコードはデプロイしたくない、しかし実際のクライアントトラフィックによるカナリアテストは行いたい・・・ということはないでしょうか。そうした場合に、クラスタをまたがったアクセスルーティング制御が必要になります。

他にもサービス間の認証やメトリクスの取得といった痒い所も随所に現れ、showKsが目指した、100以上の開発プロジェクトによる100以上のマイクロサービスというスケールを実現しようとすると、つい@<b>{「ああ・・・やっぱService Meshって必要なものなんだなあ」}という声が洩れました。

今回は「Istio@<fn>{istio}やってみたいけどそこまで手出したらさすがに死ぬ」と早い段階で諦めたService Meshですが、次回は是非チャレンジしたいです。

//footnote[istio][https://istio.io/]
//footnote[linkerd][https://linkerd.io/]
//footnote[conduit][https://conduit.io/]
//footnote[consul][https://www.consul.io/]

== Serverless

KubernetesにServerless Computingの考え方を持ち込むことができるKnative@<fn>{knative}が注目を集めつつあります。Googleを始めとしてPivotal, IBM, Red Hatなどの企業が開発を進めているこのソフトウェアは、もしかすると2019年の台風の目になるかもしれません。

今回はKnativeの特徴であるイベントドリブンなアーキテクチャを活用できそうな場所があまりなかったこと、またKnative自体が登場したばかりで安定しているとは言いがたいことから、採用は見送りました。 しかし、有望なテクノロジーであることは間違いないため、是非試してみたいなと考えています。

//footnote[knative][https://www.knative.dev/]

== アプリケーションの充実

スケーラブルで堅牢なアーキテクチャをマイクロサービスで実現してクラウドネイティブ・アプリ開発らしさを出すという目的は、今回のshowKsで概ね達成できました。しかし、カンファレンスという特殊な環境において、わかりやすい展示になることを優先させた部分は少なからずありました。そのためアプリケーションの構成が、実際の企業のシナリオとは少し異なったものとなってしまったかもしれません。

実際の企業におけるアプリ開発シーンでは、例えば：

 * コンポーネントを担当する部署が異なるので、デプロイ単位もそれに合わせて分けたい
 * リリースサイクルが異なる機能はコンポーネント化したい

といったような理由からマイクロサービス・アーキテクチャにする必然が生まれます。このようなシナリオにおいて、実際の現場で参考にしてもらえるようなサンプル・アプリケーションにするべく、題材選びは引き続き大きな課題です。

また、アプリケーションの機能面でも、マイクロサービス間の通信手段やストレージの利用など、改良・追加して試してみたいことは山のようにあります。各サービスを構築する言語などのツール選定も、トレンドに合ったものにすべきでしょう。

次の機会では、クラウドネイティブ・アプリ開発のトレンドをうまく反映して、実際の企業シナリオにすぐにでも適用できるようなデモができることを目指したいと思います。

== 監視のアラートの整備と耐障害性の向上

モニタリングについてはPrometheusやWeave Scopeである程度実現ができました。しかし今回は監視のアラート通知までは実装できていませんでした。モニタリングをすることはとても大切なことですが、ずっとモニタリングの画面を見続けるのは現実的ではありません。ですので何か障害を検知した際に通知を行い、障害にいち早く気が付く仕組みを作ることもとても大切なことです。

幸いなことに特に大きな障害は起こることなくshowKsをリリースすることができましたが、次回も同じように行くとは限りません。ですので次回はしっかりと通知する仕組みを組み込みたいと考えています。

クラウドネイティブの定義の一つに復元力というものがありましたよね？showKsはクラウドネイティブなアプリケーションですから、ちょっとやそっとの障害で動かなくなってしまっては困ります。その為には意図的にサーバーやコンテナを落としてみて、システムにどのような影響があるのか？SPOF（Single Point Of Failure）になっている部分は無いか？を常に把握しておき、意図しない本物の障害が起きたとしても被害を最小限にするようにしたいものです。このような考え方はChaos Engineering@<fn>{chaosengineering}と言われています。

本当はみなさんにデプロイしてもらったアプリケーションをランダムに落としたりしようかみたいな話も出てきましたが、結局はそこまで実現しませんでした。

今回のshowKsでは実際にクラウドネイティブな開発の体験をしてもらいましたが、多少の障害が起きても堅牢に動き続けるshowKsも体験してもらうべく、次回はChaos Monkey@<fn>{chaosmonkey}のようなツールの導入も試してみたいと考えています。

//footnote[chaosengineering][http://principlesofchaos.org/]
//footnote[chaosmonkey][https://github.com/Netflix/chaosmonkey]

===[column] 【コラム】JKD参加者にshowKsよ！届け！！

ここまでご紹介してきたように、様々な工夫と努力を経てshowKsは作り上げられました。
しかし、どれだけ良いモノが出来上がったとしても、それをJKD参加者へ届けられなければ意味がありません。
このshowKs企画の"Last 1 Mile"です。

1人でも多くshowKsに参加してクラウドネイティブな開発を体験して欲しい、その想いの為に私たちは次のような工夫をしました。

 * ランディングページの作成
 ** 参加者がスムーズに企画を理解し、迷うこと無く開発体験を開始できるように、GitHub Pagesを利用したランディングページと参加手順ページを用意しました。
 * A4ペラのパンフレット配布
 ** ランディングページへの省略URLを含むshowKs企画の図解入り説明パンフを作成し、来場者に向けて配布しました。
 * 前夜祭でのLightning Talk（チラ見せ）
 ** JKDの前日に開催された前夜祭イベントで、showKsのPRをするLTを行いました。
 * オープニングでのPR
 ** JKDのオープニングの中で、オーガナイザーである鈴木さん （@szkn27） からshowKsをPRして頂きました。
 * showKsブースでのPR
 ** ブース壁面に、ランディングページへのQRコードを含む様々なA3ポスターを張り出しました。
 * 打ち上げ会場でも・・・
 ** スタッフ/登壇者に向けて「まだ間に合います！やりましょう！」と最後のPRを行いました。

これらの努力が実り、showKsへは2日間で100名を超える参加がありました。
多彩なイラストもポータル画面に並び、盛況であったと言っても過言ではないでしょう。

//image[showks-pamphlet][配布したパンフレット][scale=0.6]{
//}

===[/column]

===[column] 【コラム】私たちのモチベーションって？

showKs企画のきっかけは第1章でもお伝えしましたが、showKsの参加メンバーは元々JKDのボランティアスタッフとして集まったメンバーが大半で、当然のことながら普段は別のお仕事を沢山抱えていたりします。

そのため、showKs開発中もミーティングは深夜にZoomのオンライン会議で、普段はSlack上でやりとりを・・・といった進め方だったりしたわけですが、そもそも私たちのモチベーションってなんだったのでしょうか？

分かりやすい”ネタ”としては、Spinnakerのところでも出てきた「モテたい」ですが、実際はモテた気がしない（多分）ですし、これはまぁ雰囲気で感じ取って貰えれば十分でしょう。

また、”ネタ”と言えば、シャリの上にのっけるものを想像しますよね。そう「寿司」です。これもJKDでのセッションスライドに出てきましたが、分かりやすいご褒美としてモチベーションを上げる一端になっていたでしょう。

しかし、本当のところはもちろん別にあったと思っています。

JKDに来てくださる皆さんに動いているものをやっぱり見せたい、クラウドネイティブってどういうことなのか肌で感じられるものが欲しい、そうした想いをメンバーが持っていたのは間違いありません。また、いざ動き始めると、会社や業務というような枠組みを超えて、コンテナ界隈の猛者達と一緒にプロジェクトを進められることが、何にも代えがたいことであると感じようになりました。まさにこの開発体験こそがプライスレス。

こうした動機や想いを抱え、日に日にshowKsプロジェクトの熱量が増していきました。結果、今回の技術書典への参加まで繋がりましたし、「次のshowKs」にも熱が引き継がれることになったのです。

余談ですが、メンバーイチ、いやKubernetes界隈イチかわいいと噂の @amsy810 が、割と海鮮は食べられないものが多いということが後から発覚し、寿司でよかったんかいな・・・とメンバー一同に動揺が一瞬走りました。だったら先に言ってくれれば・・・。

まぁ、そんなところも可愛いですね。

===[/column]

===[column] 【コラム】アポーペン、アレはいいものだ

Canvasはお絵かきアプリですが、実は当初は使用できる色も少なく、さらに消しゴム機能などもありませんでした。
@kojiha がもくもくと機能改善に取り組んではいましたが、JKDの開催直前の時期に大きな出来事がありました。

そう・・・新型のiPad ProとApple Pencilの発売です。

これは良さそうだぞと手書きスライドでお馴染みの @jyoshise が速攻で購入したのですが、iPadでも快適に使いたいよね・・・という流れの結果、最終的には筆圧まで対応したCanvasができあがりました。

JKDにご来場された方には、iPad ProとApple Pencilでお絵描きする @jyoshise の姿を見た方もいるのではないでしょうか。展示デモにも最適な1台でした。

===[/column]
