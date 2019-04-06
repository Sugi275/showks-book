= クラウドネイティブのショーケースを作る

== 当初の計画

前章で説明した全体コンセプトが決定したのが9月末頃。12月頭のJKD本番まで、あと2ヶ月とすこしというタイミングでした。決して余裕のあるスケジュールではないものの、まだこの段階では楽観視していたのです。なぜなら、やるべき事が明確であり、仕組みもシンプルであるように思えたからです。 このとき、僕らの頭の中にあるイメージは@<img>{at_first}のようでした。

//image[at_first][当初の構想][scale=0.6]{
//}

シンプル。実にシンプル。これなら1ヶ月もあれば実現できそうです。

でも、僕らのそんな淡い期待は早々に打ち砕かれることになるのです。なぜなら、実際に作り始めると、あれもこれも必要だと言うことが分かってきたのです。それでは、どういうツールをどういう考えで使ったのか、紹介していきます。


== 持ち帰りを実現するキーワード 『Infrastructure as Code』

第1章でも述べたように、showKsでは@<b>{お持ち帰り}が大きなテーマのひとつでした。お絵描きアプリで遊んで、その仕組みを説明されただけでは、なかなか知識として定着しませんよね。僕たちが作った仕組みそのものを持ち帰ってもらい、皆さんの環境上でクラウドネイティブなCI/CDを実現してもらいたかったのです。

だからといって、昔ながらの@<b>{構築手順書}には絶対にしたくなかったんです。だって@<b>{かっこ悪いもん}。じゃあどうするか？ コード化するに決まってるでしょう！ そう、Infrastructure as Code（IaC）をやれば、持ち帰りやすくなりますよね。

=== Terraform

マネージドのKubernetesを使いたい！となったとき、第一の選択肢に上がるのがGoogle Kubernetes Engine（GKE）でしょう。今回のshowKsでも、GKEを利用しました。その他、アプリケーションで利用するファイルの置き場や静的IPなども必要となりますので、以下のサービスを利用しました。

 * Google Cloud Storage
 * Google Cloud DNS
 * Static External IP
 * Google Compute Engine（踏み台サーバー用）

マネージドサービスはとても便利ですし、皆さん結構GUIを使ってポチポチ構築されているのではないでしょうか。でも、GUIを使うのは手数がかかりますし、再現性の点においてもあまり好ましくありません。そこで、これらをプロビジョニングする仕組みとして、HashiCorpのTerraformを利用しました。

ファイルはGitHub@<fn>{showks-terraform}からダウンロード可能です。

//footnote[showks-terraform][https://github.com/containerdaysjp/showks-terraform]

=== Kubernetes

アプリケーションはすべてKubernetesの上で動かします。もともとKubernetesはYAML形式で書かれた宣言的コード（マニフェスト）を用いて設定を行っていく仕組みのため、何もしなくても自動的にInfrastructure as Codeが実現できることになります。
Infrastruture as Code？Kubernetesには朝飯前です。

例えば、nginxのコンテナを3つ起動する場合は下記のようなマニフェスト記述し、Kubernetesに登録するだけでその状態に維持し続けてくれます。
そのため、Kubernetesでアプリケーションのアップデートを行う際には、下記のマニフェストの17行目の「nginx:1.12」を「nginx:1.13」のようにイメージタグを変更して再登録することで利用するコンテナイメージ（アプリケーション）のアップデートを行うことになります。変更手順はとっても簡単ですね！

//emlistnum[][yaml]{
apiVersion: apps/v1
kind: Deployment
metadata:
  name: sample-deployment
spec:
  replicas: 3
  selector:
    matchLabels:
      app: sample-app
  template:
    metadata:
      labels:
        app: sample-app
    spec:
      containers:
        - name: nginx-container
          image: nginx:1.12
          ports:
            - containerPort: 80
//}

Kubernetesの利用者はこの宣言的なマニフェストの書き方を学ぶだけで、あとはKubernetesが定義された状態に自動的に調整してくれる仕組みになっています。
CI/CDの整備をする際にはこのマニフェストをどのように利用するかが肝心になってきます。

=== Helm

Kubernetesのマニフェストを書いていくと、同じようなYAMLファイルをたくさん書く必要が出てきますね。
しかし、大量のマイクロサービスが作られる環境では共通する部分も多く、エンジニアなら誰しも上手くテンプレート化することで記述量を削減したくなってきます。
マニフェスト作成を助けるツールはいくつかありますが、今回はその中でも最も有名なHelmを採用しました。Helmはパッケージマネージャーとして知られており、 "helm install" コマンドで様々なアプリケーションを簡単にKubernetesにデプロイすることが出来ます。
しかし、今回はパッケージマネジメントの仕組みは使わず、純粋なテンプレートエンジンとして利用しています（ここ重要）。

例えば通常のWebアプリケーションを動作させる場合、コンテナを起動させるDeploymentリソースとエンドポイントを作るServiceリソースを同時に作成することが多いかと思います。
こういった際にはマイクロサービスごとに同じような大量のマニフェストを定義しなければならず、変更漏れなどの可能性も出てきてしまいます。
Helmでは下記のようなテンプレートとValuesファイルを利用することで、マニフェストのテンプレーティングを行うことが可能です。

//listnum[yaml][Helmテンプレートの例]{
apiVersion: apps/v1
kind: Deployment
metadata:
  name: {{ .Values.appName }}-deployment
spec:
  replicas: {{ .Values.replicas }}
  selector:
    matchLabels:
      app: {{ .Values.appName }}
  template:
    metadata:
      labels:
        app: {{ .Values.appName }}
    spec:
      containers:
        - name: {{ .Values.appName }}-container
          image: {{ .Values.image.repository }}:{{ .Values.image.tag }}
          ports:
            - containerPort: {{ .Values.port }}
//}

//listnum[yaml][valuesファイルの例]{
appName: my-nginx
replicas: 1
image:
  repository: nginx
  tag: 1.12
port: 80
//}


showKsでも、Helmを利用してマニフェストを生成することで再利用性やマニフェストの生成処理を管理しやすくしています。@<fn>{showks-canvas-helm}

//footnote[showks-canvas-helm][https://github.com/containerdaysjp/showks-canvas/tree/master/helm]

//image[iac][Infrastructure as Codeで環境をプロビジョニング][scale=0.5]{
//}

== CI/CD

クラウドネイティブ、マイクロサービス、アジャイル開発、といった今どきのイケてる風な方法論を語る上で、CI/CDは必須のキーワードとなっています。CNCFのCloud Native Trailmap@<fn>{trailmap}でも、Containerizationの次のステップとしてCI/CDが位置付けられており、「いかにCI/CDを効率よく回すか」がクラウドネイティブの主たる目的のひとつであると言っても過言ではありません。

//footnote[trailmap][https://github.com/cncf/trailmap]

と偉そうに言ってみたものの、僕らは「クラウドネイティブなCI/CD」とは何なのか、ちゃんと定義できているのでしょうか？

CI/CDのパイプラインを経て生成される成果物、および中間成果物は、一般的にはArtifactと呼ばれます。Javaを例に取ると、.warファイルがこれに当たりますね。

ちょっと待って。僕らがやろうとしているのは「クラウドネイティブな」CI/CDであり、なにも.warファイルをweblogicサーバーにデプロイしたいわけではありません。
アプリケーションはコンテナとして動くことが前提ですし、デプロイ先のインフラはKubernetesが前提です。その場合、CI/CDパイプラインが生成するArtifactは何になるでしょう？

一つの有力な候補は、Docker Imageです。Docker ImageをArtifactとするなら、CI/CDにおけるビルドタスクの一つとしてコンテナイメージのビルド（docker build）が実行されることになり、生成されたDocker Imageは何らかのイメージレジストリ（DockerHubなど）でバージョン管理することになります。
これであれば、.warファイルをNexusで管理する、とかと大して違いませんね。

ですが、それだけでしょうか？ArtifactがDocker Imageだけだとすると、Kubernetes上にはそれだけではデプロイできません。

前述の通り、Kubernetesはアプリケーションの実行状態をマニフェストというYAMLファイルで定義するものであり、今回はそのマニフェストはHelmにより生成されます。
つまり、個々のアプリケーションにとってのInfrastructure as Codeであるマニフェストもまた、アプリケーションのビルド成果物ということになります。

このように、アプリケーションのバイナリ及びそれを含むDocker Imageだけではなく、その実行状態を定義するInfrastructure as Code自体をArtifactとして取り扱うことが、クラウドネイティブなCI/CDパイプラインと言えるのではないでしょうか。


===[column] 【コラム】CIとCDの境界線

本来、CI=Continuous Integrationの目的は、複数の開発ブランチが並行して走るプロジェクトにおいて、コンフリクトを解消しマージを省力化するための自動化です。
マージされたレポジトリから、本番環境にデプロイできる状態のビルド成果物を生成するのがCD=Continous Deliveryとなります。

ですが、特にコンテナやマイクロサービスの文脈でCI/CDを語る際、CIは「アプリケーションのビルドの自動化」で、CDは「デプロイの自動化」を意味することが多いように思います。つまり、

 * 誰かがソースコードをプロジェクトのRepositoryにコミットする
 * CIツールがその変更を察知して、必要なタスクを実行する
 * ビルドが自動実行されて、最新のビルド成果物が然るべき場所に格納される

という一連の流れが、常に回っている状態（Continuous）がCIであり、

 * CIの成功などをトリガとして、CDツールがビルド成果物を実行環境にデプロイする

というのが常に回っている状態がCD、というイメージです。

この場合のCDはContinuous Deliveryの略ではなく、Continuous Deploymentの略である、という説もあるようです。

まあ、その定義にこだわることに大した意味はなくて、要はソースコードからリリースまでのパイプライン全体を考えて、適材適所のツールを使う、ということが大事なのかもしれません。

===[/column]


=== Concourse CI

今回はCIツールとしてConcourse CIを利用しました。
Concourse CIではCI PipelineをYAMLで定義することが可能なため、こちらもコード化を行うことが可能です。あらやだ素敵。
また、Helmを使ってKubernetes上に簡単にデプロイしたり、BOSHを使ってVM上に簡単にデプロイすることも可能な点も良いところですね。あらやだ素敵。

TODO
（全体的に説明の追加要）


利用したPipeline YAMLはGitHub@<fn>{showks-concourse-pipeline}からダウンロード可能です。

//footnote[showks-concourse-pipeline][https://github.com/containerdaysjp/showks-concourse-pipelines]

=== Spinnakerでモテたい

Spinnakerというツールはご存じでしょうか。
触ったことはないけど、名前だけは知っているという方も多いのでは無いでしょうか？
本家であるNetflixをはじめとして、イケてるクラウドネイティブ企業が大規模なサービスのリリースに活用しており、Spinnakerという名前の響きもなんだかかっこよくて、「俺Spinnaker使ってるんだ」というとモテそうです。

Spinnakerは、Continuous Deliveryに特化したツールであり、アプリケーションのビルド部分については別途CIツールを利用することが前提となっています。
また、ConcourseのようなCIツールはデリバリーまで含めたパイプラインを実行することも可能なのですが、やはりショーケースとしてこの人気ツールを外すことはできないでしょう。
@<b>{だってモテたいし}。というわけで、ツールがひとつ増えました。

NetflixがSpinnakerをオープンソース化したのは2015年です。
最近のようですが、当時はマルチクラウドに対応したアプリケーションのデリバリといえば、IaaSに対応する仮想マシンイメージのビルドとデプロイ、あるいはPaaSへのデプロイを意味していました。
そのため、従来のSpinnakerの機能は、EC2のようなIaaSや、Google App EngineのようなPaaSをターゲット（Cloud Provider）として、CIが生成するArtifactをデリバリするというものでした。

その後Kubernetesが急速に発展し、Infrastructure as CodeであるマニフェストをアプリケーションのArtifactとして扱うという考え方が非常に有効であることが認識されるようになりました。
そこで2018年になって、SpinnakerにもKubernetes V2 Provider（Manifest Based）が登場しました。
これはSpinnakerにとっては、個々のアプリケーションに対するインフラストラクチャの定義を、Spinnakerの設定ではなくアプリケーションのArtifactとして取り扱うという意味で、大きな変換点となります。

showKs企画時点では、Kubernetes V2 Providerはまだbeta扱いでした。
でもこの先進的なコンセプトを使わない手はないでしょう。
@<b>{だってモテたいし}。というわけで、迷わずKubernetes V2 Providerを使うことに決定。

SpinnakerはNetflixが作っただけあって、それ自体がマイクロサービスアーキテクチャで作られています。
Spinnakerのデプロイ方法はいろいろあるのですが、今回はせっかくなのでKubernetesクラスタ上にデプロイすることにしました。
デプロイヤーとしては専用のHalyardというツールがあり、Spinnakerのデプロイ構成や各種設定は単一のYAMLファイルとして管理することができます。
ここでもInfrastructure as Code。

また、パイプラインの設定をWeb UIからポチポチやらなければならない、つまりパイプライン定義がImmutableでないことがSpinnakerのイケてないところと言われていましたが、最近ではいちおう定義されたパイプラインをJSONで吐き出したり、JSONで記述したパイプラインを読み込んだり、といったことが可能になっています。
ただ、そのためのCLIツールであるspinの機能はこの時点ではかなり限定されていて、テンプレートからの生成といったことはできませんでした@<fn>{spinnaker-json}。
そのためshowKsでは、テンプレートとなるJSONを手書きして、そこにPerl置換でパラメーターを埋め込むという方法をとりました。

//footnote[spinnaker-json][本書の執筆時点ではできるようになったようです。https://www.spinnaker.io/guides/spin/pipeline-templates/]


SpinnakerのPipeline JSONはGitHub@<fn>{spinnaker-pipelines}からダウンロード可能です。


//footnote[spinnaker-pipelines][https://github.com/containerdaysjp/showks-spinnaker-pipelines]


== GitOpsでアプリケーション開発者との親和性も

様々なツールがありますが、アプリケーション開発者はGitHub上のソースコードを更新することで開発を進めていきます。
そんなバックグラウンドと非常に相性が良いと思うのがGitOpsです。ChatOpsのGit版ですね！

GitOpsはWeaveworksが提唱するContinious Deliveryを実現する手法の一つです。@<fn>{gitops-weaveworks}
GitOpsではContinious DeliveryをGitリポジトリを介して行うことにより、全ての変更はPull-Requestをベースに行われていきます。
GitOpsで登場するリポジトリは2種類存在します。一つはアプリケーションのソースコード用のリポジトリ、もう一つはKubernetesのマニフェスト用のリポジトリです。
流れとしては、まずアプリケーション開発者はアプリケーションのリポジトリに対してPull-Requestを作成後、UnitTest・コンテナイメージのビルドなどのContinuous Integrationが行われます。
ここまでは従来のアプリケーション開発と何も変わりません。
GitOpsではこのCIが行われた際に、マニフェスト用のリポジトリに対して更新を行います。より具体的にはマニフェストに記述された利用するコンテナイメージ（spec.containers[].image）のタグ（バージョン）を変更します。
その後リポジトリのマニフェストが更新されたことを検知すると、実際にKubernetesクラスタに対して変更が加えられるようになります。

//image[gitops][GitOpsの実践][scale=0.6]{
//}

GitOpsはあくまでも概念であり、マニフェストの更新や更新時のKubernetesクラスタへの適用といった実装は多岐に渡ります。
GitOpsの実装としては、Jenkins X@<fn>{jenkinsx}やWeave Flux@<fn>{flux}などがありますが、今回はHelmを利用したマニフェスト生成とSpinnakerを利用しています。@<b>{だってモテたいし}。

//footnote[gitops-weaveworks][https://www.weave.works/technologies/gitops/]
//footnote[jenkinsx][https://github.com/jenkins-x]
//footnote[flux][https://github.com/weaveworks/flux]

GitOpsを採用することにより、多くのメリットがあります。
1つはKubernetesのマニフェストをGitリポジトリに保管しておくことにより、Kubernetesの状態に関してSingle Source of Truthを実現することが可能な点です。
システム全体の状態がマニフェストとしてGitリポジトリ上に管理されているため、障害時には再度宣言的に記述されたマニフェストを適用することにより迅速に回復させることが可能になります。
また、Kubernetesへの変更は全てGitリポジトリを介して行われるという特性上、変更の監査履歴としても利用することが可能です。

他にもアプリケーション開発者からみたときに、Kubernetesを意識する必要がほとんど無くなります。
開発者はあくまでもアプリケーションのGitリポジトリに対して変更を加えればよく、そのあとのコンテナをKubernetesへデリバリするのは自動化されたGitOpsの役割になります。
そのため、開発者の生産性を損ねることなく、コンテナの軽量さを生かして生産性やアプリケーションのリリース頻度を高めることが可能です。


===[column] 【コラム】膨らむ構成

シンプルだなんて言ってたけど、だんだん雲行きが怪しくなってきましたね。
この段階で、当初の計画よりも2倍くらい要素が増えてしまいました。アプリケーションだけならまだシンプルだと言えますが、それを上手く運用していくためのツールや手法がとにかく多い。
とはいえ、これらツールの支援無しにはクラウドネイティブな開発が出来ないのも事実。このあたりのツラミについては、第3章にて詳しく説明します。

一方でこのCI/CDの仕組みを一度完成させると、クラウドネイティブの定義で紹介した下記のような利点を享受することができます。
組織の開発力を向上させるためにも、プロジェクトを始める前にしっかりとしたCI/CD環境を整備するのは大切なことなのです。

 * 疎結合なシステム
 * 復元力がある
 * 管理しやすい
 * 堅牢な自動化により、頻繁かつ期待通りに最小限の労力で大きな変更が可能


===[/column]

===[column] 【コラム】涙無しには語れないJenkins X裏話

CI/CDツールとして真っ先に頭に浮かぶのはJenkins@<fn>{jenkins}という方も多いかもしれません。確かにJenkinsはとても便利です。
そしてKubernetesと連動して動くJenkins X@<fn>{jenkinsx}というツールも最近では登場してきました。

Jenkins Xはクラウドネイティブアプリケーションを開発するためのKubernetesネイティブな継続的インテグレーションおよび継続的デリバリプラットフォームです。
Jenkins Xでアプリケーションを管理すると、アプリケーションがHelmでパッケージングされてKubernetesに展開する準備をしてくれます。
利用者がアプリケーションを更新すると、その更新を検知してJenkins XはそのプレビューサイトをKubernetes上のstagingのnamespaceに用意し、本番環境へのPull Requestを作成してくれます。
確認したらPull Requestをmergeするだけでアプリケーションを本番環境に展開することができます。ここらへんはGitOpsの概念が取り入れられています。

このJenkins Xですが、当然showKsの構想では名前が上がっていました。まさにうってつけのツールなのですが、、、ちょっと待って下さい。これってほぼ自分たちがやることが無くなるのではないのか・・・？

そうなんです。Jenkins Xを採用すれば楽になれたのかもしれません。しかしshowKsのメンバーにとってはそんなことはどうでもいいのです。Spinnaker使ってモテたい、ただそれだけなのです。

そんなわけで実は裏でいろいろ検証をしていたJenkins Xはそっと閉じられることになったのです。次回は是非使ってみたいですね。

//footnote[jenkins][https://jenkins.io/]
//footnote[jenkinsx][https://jenkins-x.io/]

===[/column]

== ユーザー体験の設計

ツール決めだけで無く、ユーザー体験の面も結構な時間を割いてディスカッションしました。今回のコンセプトのひとつが『クラウドネイティブな開発の体験』なんですから、そこをおざなりにすることは出来ません。
JKDの運営委員会の後にみんなで残って、やんややんやとディスカッションを行ったことを覚えています。

//image[ux-discussion][ユーザー体験についてのディスカッション@インプレス][scale=0.6]{
//}

参加者は、アプリケーションの開発者になった気持ちになってもらい、コードを書いた後GitHubにpushしてもらいます。
もちろん、イベント中に一からコードを書くのは難易度が高い話になってしまいますので、代わりの仕組みとしてJSONを編集すると、その情報がキャンバスに反映されるようにしました。

ブランチ戦略としてはstagingとmasterを準備。本当の開発だとfeature-staging-masterと3段階くらいに分かれているケースが多いのではないかと思いますが、今回はイベント中の体験重視で2段構えのブランチとしました。

stagingにpushされたコードは、Concourse CIでユニットテストが行われます。InvalidなJSONであったり、JSON以外の編集があった場合はテストがコケるようになっています。
JSON以外のコードを受け付けないようにしているのは、悪意のあるコードをshowKs環境で動かされるのを防ぐためです。

ユニットテストが通過したら、コンテナイメージのbuildとタグ付け、レジストリへのpushを行います。その後、KubernetesのマニフェストをHelmテンプレートから生成し、デプロイします。
これでStaging環境で動作確認が行えるようになります。

その後、開発が終わったとの想定でstagingからmasterブランチに対してPull Requestを投げて貰います。
Concourse CIがそれを検知し、不正なリポジトリからPull Requestが送られていないかを確認し、テストした後問題が無ければLGTMのコメント後、マージを行います。

これでmasterブランチに変更が取り込まれましたので、その変更を再びConcourseが検知し、Stagingで作成したイメージへのタグ追加とマニフェスト生成を行います。
最後にSpinnakerがデプロイを行って、Productionリリースが完了となります。フローを図にしたものが、@<img>{developer-experience}です。
シンプルさを重要視した結果、GitHub Flow@<fn>{github-flow}と呼ばれるブランチ戦略とほぼ同じ仕組みになりました。

//footnote[github-flow][http://scottchacon.com/2011/08/31/github-flow.html]

//image[developer-experience][デベロッパーワークフロー][scale=0.8]{
//}

== 本番を想定するならば、少なくとも2面は環境必要だよね

ディスカッションしている間に、こんな話も出てきました。『パイプラインをProductionとStagingに分けて作ってるけどさあ、環境自体はどうする･･･?』

確かに僕らは今までProduction・Stagingを区別してコードを書いてきましたが、実際にアプリケーションを載せるKubernetesのほうは、特に何も考えていなかったのです。
でも、本当にProductionに出すとすると、Kubernetesクラスタは分けるケースが多いのではないでしょうか。クラウドネイティブのショーケースを名乗るのであれば、やっぱりここも分けた方がいいかもしれません。

Kubernetesクラスタが分かれるということは、GitOps的にもリポジトリが分かれます@<fn>{showks-manifests-prod}@<fn>{showks-manifests-stg}。
そして、それをデプロイするSpinnakerのパイプラインも分かれることになります。図にすると@<img>{separate-env}のようになるでしょうか。 やばい、どこまで構成大きくなるんだろう。

//image[separate-env][ProductionとStagingの分離][scale=0.6]{
//}

//footnote[showks-manifests-prod][https://github.com/containerdaysjp/showks-manifests-prod]
//footnote[showks-manifests-stg][https://github.com/containerdaysjp/showks-manifests-stg]

== 申し込みフォームいるじゃん！どうしよう

ここにきて大きな考慮漏れに気づいてしまいました。いや、あえて思考を後回しにしていたとも言えるのですが･･･ 参加型企画なのだから、@<b>{サインアップの仕組み}が必須なのです。
何らしかの方法で、参加者から必要な情報を渡して貰う必要があります。

今回、参加者にはGitHubを使ったPull Request開発を体験してもらいますので、GitHubアカウントは必須になります。また、キャンバスに掲載したい情報も渡して貰いたいですね。
つまり、以下のような項目の入力が必要なわけです。

 * ユーザー名（必須）
 * GitHubアカウント名（必須）
 * TwitterID （オプション）
 * コメント（オプション）

当初は、Google Formsを使ってさっくりと用意しようと考えていました。しかし、よくよく考えてみると以下のような制約があることに気づきます。

 * ユーザー名はKubernetesのリソース名に使われるため、ユニークでなくてはならない
 * KubernetesのServiceとしてもユーザー名が使われる。名前解決にも利用されるため、利用できる文字が限られる（例えばアンダースコアはNG）
 * GitHubにもInvitationを送る必要があるため、ValidなGitHubアカウントでなくてはならない

つまり、ちゃんとしたバリデーションが必要ということになります。ここまでのバリデーションは、Google Formsでやるのは難しそうです。

最終的に、このフォームはRuby on Railsで書くことになりました。showks-formという名前で、独立したアプリケーションとして動かすことにします@<fn>{showks-form}。

RoRを採用したことで、ActiveRecordの持つ強力なバリデータによって、たった数行@<list>{project.rb}でバリデーションを行うことができました。

//listnum[project.rb][バリデーション部分][ruby]{
class Project < ApplicationRecord
  include ActiveModel::Validations
  validates_with GitHubUserValidator
  validates :username, uniqueness: true, presence: true,
            format: { with: /\A[a-z0-9\-]+\z/}, length: { maximum: 30 }
  validates :github_id, uniqueness: true, presence: true, length: { maximum: 30 }
  validates :twitter_id, format: { with: /\A[a-zA-Z0-9\_]+\z/}, length: { maximum: 15 }
  validates :comment, length: { maximum: 100 }
（略）
//}

//footnote[showks-form][https://github.com/containerdaysjp/showks-form]

=== 爆誕 Pipeline as Code

申し込みフォームも目処がついたところで、いよいよユーザー申し込みからキャンバスの構築まで具体的なイメージが沸くようになってきました。

ユーザーがフォームを入力後、一連の流れを図に起こしてみると@<img>{showks-form}のようになります。

//image[showks-form][フォーム入力後の処理][scale=0.6]{
//}

かなり沢山の処理があることが分かりますね。大きく分けると『GitHubの設定』『Concourseの設定』『Spinnakerの設定』の3つになります。

GitHubの操作は、GitHub APIを叩くGemであるOktokitを利用しました。リポジトリの作成だけでなく、WebhookやCollaborator、Protected Branchの設定まで全てこのGemで設定できたので、とても助かりました。

//listnum[octokit][Oktokitを使ってGitHubにリポジトリを作成する例][ruby]{
  def create_repository
    @client = Octokit::Client.new(
      login: Rails.application.credentials.github[:username],
      password: Rails.application.credentials.github[:password])
    if @client.repository?("containerdaysjp/#{repository_name}")
      @repo = @client.repository("containerdaysjp/#{repository_name}")
    else
      @repo = @client.create_repository(
        repository_name,
        {organization: "containerdaysjp",
        team_id: 3013077})
    end
（略）
//}

Concourseは標準のCLIであるflyコマンド、Spinnakerはspinコマンドを、Railsから実行するという形の実装となっています。

これらの処理が、ユーザーの申し込みごとに行われることになります。1ユーザー申し込みごとに、GitHubのリポジトリが1つ、Concourseのパイプラインが3つ、Spinnakerのパイプラインが2つ出来上がります。
もしamsy810, jacopen, jyoshiseの3ユーザーが申し込んだとすると、作成されるリソースは@<img>{provisioned-resources}のようになるわけですね。

//image[provisioned-resources][作成されたリソース][scale=0.6]{
//}

===[column] 【コラム】Pipeline as Code

僕たちはこの仕組みのことを、Pipeline as Codeと呼ぶことにしました。showKsならではの仕組みで一般の開発に使える余地がどのくらいあるかは分かりませんが、たとえば組織内でリポジトリやCI/CDパイプラインを標準化して、プロジェクト開始時にこれらを一発でプロビジョニングする、といったことをしたい場合には有用な考え方かもしれません。

===[/column]



== カナリアリリース

カナリアリリースとはリリース手法の1つで、サービスを一部のユーザだけが利用できるように段階的にリリースする手法です。
例えば、新しいバージョンのWebアプリケーションをリリースする際に一気に切り替えてしまうと、問題があった場合に影響が非常に大きくなってしまいます。
カナリアリリースでは、例えば最初は新しいバージョンには総トラフィックの1%だけを転送するようにして、徐々に新しいバージョンに転送するトラフィックの量を5%・10%と徐々に増やしていく方法をとります。
小規模にリリースすることにより、問題が発生した際には影響範囲が小さいうちに現行バージョンにロールバックすることが可能です。

Kubernetes環境でカナリアリリースを行うにはいくつか方法があります。
一番簡単な方法はServiceリソースのspec.selectorをうまく使うことで、複数のバージョンのDepolymentやPodに対してリクエストを分散する方法です@<fn>{deployment-canary}。
この方法では1%だけリクエストを新しいバージョンに転送したい場合には、最低でも100Podが必要になってしまい細かい制御などはできません。
//footnote[deployment-canary][https://kubernetes.io/docs/concepts/cluster-administration/manage-deployment/#canary-deployments]

他にはIstioのVirtualServiceを利用する方法もあります@<fn>{istio-canary}。
Istioでは特定のCookieが付与されたリクエストだけを新しいバージョンに転送したり、より細かなトラフィック制御を行うことも可能です。
//footnote[istio-canary][https://istio.io/docs/tasks/traffic-management/traffic-shifting/]

ここまでKubernetes上での2種類のカナリアリリースの実現方法を紹介しましたが、今回showKsではkubernetes/ingress-nginx@<fn>{ingress-nginx}（いわゆるNginx Ingress Controller）を用いたカナリアリリースを利用しました。
//footnote[ingress-nginx][https://github.com/kubernetes/ingress-nginx]

今回CDツールとして採用したSpinnakerですが、カナリアリリースがSpinnakerの「売り」の一つ、という印象をお持ちの方も多いのではないでしょうか。
実は今回もSpinnakerによるカナリアリリースを実装しようとしていたのですが、結論としては、SpinnakerはKubernetes V2 Providerへの移行の過渡期ということもあり、ArtifactとしてKubernetes Manifestsを利用する場合はうまくカナリアリリースができないことがわかりました。

今回、アプリケーションのソースコードは本番リリース前のブランチ（Stagingブランチ）と、本番リリース用のMasterブランチに分かれています。KubernetesクラスタもStagingクラスタとProductionクラスタに分かれており、アプリケーションを定義するManifestもStagingクラスタ用のManifestとProductionクラスタ用のManifestに分かれることになります。

やりたかったことは、

 1. GitのStagingブランチにコードがcommitされる
 2. ビルド後、StagingクラスタにManifestが適用される（Podがデプロイされる）→開発者による動作確認
 3. StagingブランチからMasterブランチにPRが発行される
 4. Stagingクラスタに「カナリア」がデプロイされ、Productionクラスタで稼働中の1バージョン前の本番Podへのトラフィックの一部が流れる
 5. カナリアのパフォーマンスに問題があった場合はカナリアを殺し、PRを却下
 6. カナリアのパフォーマンスに問題がなければ、StagingブランチをMasterブランチにマージする（→ProductionクラスタのManifestがバージョンアップされる）

といったものでした。@<img>{canary1}はこれを議論していた時の落書きです。なかなか複雑ですよね・・・

//image[canary1][やりたかったカナリアリリース][scale=0.8]{
//}

ところが現状のSpinnaker Kubernetes V2 Providerでは、クラスタをまたがる形でArtifact（つまりManifest）を引き継げないのです。

さて困った・・・と言っていたのがJKDの2週間前。何か代替手段はないかと探していたところ、kubernetes/ingress-nginxに"Add canary annotation and alternative backends for traffic shaping"というPR@<fn>{pr-canary}がマージされていることを発見。これが含まれたv0.21がリリースされたら行けそうだ、とドキドキしながら待っていたのですが、無事JKDの10日前にリリースされました。

//footnote[pr-canary][https://github.com/kubernetes/ingress-nginx/pull/3341]

このingress-nginxの機能により、Ingressのannotationとして下記のように記述することで、同じhostnameとportを持つサービスへのトラフィックの一部（上記の例では20%）を転送するIngressが定義できるのです。

//listnum[ingress.yaml][カナリアリリース用Ingress定義の例][yaml]{
apiVersion: extensions/v1beta1
kind: Ingress
metadata:
  name: showks-portal-frontend-temp
  namespace: showks
  annotations:
    kubernetes.io/ingress.class: "nginx"
    nginx.ingress.kubernetes.io/canary: "true"
    nginx.ingress.kubernetes.io/canary-weight: "20"
    nginx.org/ssl-services: "showks-portal-frontend-temp"
    ingress.kubernetes.io/ssl-redirect: "true"
（略）
//}

意図していた、「カナリアはStagingクラスタ、正式リリースはProductionクラスタ」ということはできなくなりますが、まあこれはこれでありでしょうということで、@<img>{canary2}のような形になりました。

//image[canary2][ingress-nginxによるカナリアリリース][scale=0.8]{
//}



===[column] 【コラム】カナリアリリース、そもそもリリースをどう考えるか

カナリアリリースは、炭鉱夫が有害な一酸化炭素や有毒ガスを検知するためにカナリアを鳥かごに入れて連れていったことに由来すると言われています。
カナリアは人間よりも有毒ガスなどに敏感なため、人間よりも早く検知することができるため、早く逃げることができるようになります。
カナリアリリースは一部のユーザに対してリリースを行い問題にいち早く気づいてもらうためのリリース手法といえば、イメージも掴みやすいのではないでしょうか。

また、従来は十分にテストをしてからリリースする傾向でしたが、近年マイクロサービス化を含むクラウドネイティブ化された状況ではカナリアリリースなどを利用してリスクを低減することで、本番環境へのリリースを気軽に行えるようにもなってきたなと感じています。
リリースが早まることで、新しい機能をいち早くユーザに届けることができるのもクラウドネイティブがなし得ることですね。

カナリアリリースのようなリリース戦略と、CI/CDパイプラインによる自動化、さらにはマイクロサービスアーキテクチャとアジャイル的な開発手法を活用することで、各開発ブランチへのコミットのサイクル、つまり開発者のクリエイティビティと、プロダクションへのリリースのサイクルを近づけることが可能になります。CI/CDパイプラインとリリースマネジメントをセットで設計することは、アジリティのキモと言えるのではないでしょうか。

===[/column]


== マイクロサービスらしいアプリケーションとは

マイクロサービスマイクロサービスと世間では囃し立てられますが、そもそもマイクロサービスの定義は何なのでしょうか。

マイクロサービスは、「サービス」という単位でビジネスロジックや各機能を疎結合な形で複数に分割し、それらを組み合わせることで大きなアプリケーションを作り上げる開発手法です。各サービスの独立性が求められるため、Dockerコンテナを使ってあげることで論理的な要素の分離と素早いサイクルでの更新に耐えうるシステムが構築できます。また、複数の別の機能を持つサービスが同時に走るような構成を作り上げる上で、Kubernetesという分散システムを使った基盤は非常にその特性と相性がよいとされており、「マイクロサービス」と「Kubernetes」はしばしば同じ文脈で用いられます。

プロジェクトをはじめたきっかけとして「クラウドネイティブなインフラを作って、参加者の人に面白さを伝える」という思いが僕たちにはありました。そのため、インフラの構想はどんどん膨らむ一方、アプリケーションの中身やその設計についてはあまり深く考えられていませんでした（正確に言うと、アプリケーションの実装を実際に行う人が少なかったとも言えます）。そして、あれこれ手を動かしながら試行錯誤を繰り返す中で、次第に以下のような形に収束していきました。

 * 参加者は1つの独立したサービスを持ち、それを自分で管理できる（マイクロサービスのプロジェクトオーナとなるイメージ）
 * 各マイクロサービスは参加者のアクション（showKs form）を起点として動的に払い出される
 * 各サービスは共通のエンドポイントを持ち、それをアグリゲーターによって集約して可観測な状態にする
 * アグリゲーターはKubernetesのAPIを使って後ろにいる各サービスたちを常に監視していて、サービスが増えたり減ったりしたときにはそれがリアルタイムで更新される
 * クライアントに提供されるアプリはアグリゲーターとだけ通信を行い、得られた結果をクライアントに伝える

こうして、ユーザーに提供するためのサービスである「canvas」、通信を集約する「aggregator」、そして、各サービスのステータスを可視化する「portal」の3つのアプリを作成することになりました。

== Kubernetesらしさを出したくて

通信を集約する「aggregator」には、全部のマイクロサービスを把握するという重要な役割があります。各マイクロサービスは、デプロイされた時点で親である「aggregator」に自分の情報を通知する、普通だったらこんな風に作りますよね。

//image[aggregator1][普通の作り方][scale=0.5]{
//}

でもアプリチームは考えました。

 * 各マイクロサービスには、インフラのことを知らないままでいてほしい（世間ではplatform independentという）
 * なにより「Kubernetesらしさ」を出したい！

入れられるなら可能な限り自分の趣味を入れる・・・ ではなくて、どうやったら皆さんのクラウドネイティブ／Kubernetes理解の役に立てるのかを考えるのが担当の使命です。こうして、やや強引にも


@<b>{「aggregator」はKubernetes APIを使ってマイクロサービスの情報を取得する}


という仕様に落ち着いたのです。

//image[aggregator2][Kubernetes APIでCanvasインスタンスを監視][scale=0.5]{
//}

使ったのはKubernetes公式サポートのJavaScriptライブラリ。非常によくできたライブラリで、Kubernetes APIのほぼ全機能をカバーしているようです。ただし使い方はソースコードに書いてあるからね、JavaScriptだし大丈夫だよね、のまさにフルサポート（泣。こうして、人知れずAPI呼び出しのデバッグに明け暮れる、というより暮れて明ける夜が続いたのでした。

真面目な話、「Watch」を使ったインスタンスのデプロイ状態の監視とアトリビュートの取得処理は、Kubernetes APIを使った自動化の参考にしていただけることと思います。showks-aggregatorのソースコードもお見逃しなく。

//footnote[showks-aggregator][https://github.com/containerdaysjp/showks-aggregator]

//listnum[app.js][CanvasアプリのインスタンスをWatchで監視する部分][js]{
function watchService(resourceVersion) {
  console.log(`Start watching services from resourceVersion: ${resourceVersion}`);
  let watch = new k8s.Watch(kc);
  let req = watch.watch(
    k8sApiEndpoint,
    {
      labelSelector: 'class=showks-canvas',
      resourceVersion: resourceVersion
    },
    (type, obj) => {
      try {
        if (type == 'ADDED') {
          console.log('New instance was added');
          addInstance(obj);
（略）
//}

== 運用無くして何がクラウドネイティブだ。 モニタリングツールを考えよう

第1章でクラウドネイティブとは何かの定義をお伝えしました。その中には管理のしやすさや、観測ができることといったポイントがありましたが、showKsにおいてもやはりクラウドネイティブとして、管理/運用に適した監視の為の仕組みを用意するべきでしょう。

Kubernetesで利用できる監視の仕組みも今や様々なものがありますが、クラウドネイティブな監視ツールと言えば、やはりPrometheus@<fn>{prometheus}とGrafana@<fn>{grafana}が基本になるのではないでしょうか。showKsにおいても今回はPrometheus/Grafanaを実装して監視できるようにしました。

すでにPrometheus/Grafanaについては認知度も十分にありますので、読者の皆様でもお使いになっている方は少なくないかも知れません。元々、PrometheusはKubernetesの元となったGoogle社のBorgでの監視ツール：Borgmonにインスパイアされて作られました。そのためKubernetesとの相性も良く、さらに可視化ツールであるGrafanaと組み合わせることで、メトリクスの収集から必要な情報の抽出、そして運用に合わせたダッシュボード作成やグラフ化などまで、とても効率良く行えます。

@<img>{grafana}は、showKs環境のNodeにおけるリソースモニタリングを表しています。今回の企画ではクラウドネイティブな開発体験をしてもらうのが主旨だったので、ダッシュボードの作り込みなどはほとんどしておりませんが、こうしたツールと連携して運用管理もしていけるんだよーということはお伝えできたのではないでしょうか。
//image[grafana][Grafanaのダッシュボード][scale=0.8]{
//}

//footnote[prometheus][https://github.com/prometheus/prometheus]
//footnote[grafana][https://github.com/grafana/grafana]

== マイクロサービスの可視化

マイクロサービスはメリットばかりではありません。細かいサービスが沢山作られるわけですから、疎結合されている各サービス全体の繋がりを把握することはマイクロサービス化によって難しくなってしまいます。

そこで出てくるのが、マイクロサービスの可視化です。どのサービスがどこと繋がっているのか、またそれぞれのサービスの構成はどのようになっているか・・・そのような情報を俯瞰的に見やすく表示し、必要に応じてドリルダウンして情報を追っていけるような仕組みも運用までを視野に入れた場合は導入を検討するべきでしょう。

当初の計画では可視化ツールについてはあまり話題に上がっていませんでしたが、JKDのブース展示では来場者に可視化した様子をお伝えできればと @jyoshise が一晩でやってくれました。@jyoshise の j は ジェバンニ の j です。ちなみに導入は12月1日のことでした。まさに3日前。

今回、可視化ツールとして採用したのはオープンソースとしてWeave Works社@<fn>{weave-works}が開発を進めている「Weave Scope」@<fn>{weave-scope}です。

展示ブースにはモニターも準備しましたので、全体像が気になった来場者には@<img>{weave-scope}のような画面を見せながら、各サービスの繋がりなどもイメージして頂けました。ただ、可視化の部分については用意して見せられる状態にしたものの、これをどう活用するのかという部分まではあまり掘り下げてお伝えすることまでは、さすがにこの短期間では手が回りませんでした。次への課題として、また考えていければなぁと思っています。

//image[weave-scope][Weave Scopeによって可視化された様子][scale=0.8]{
//}

//footnote[weave-works][https://www.weave.works]
//footnote[weave-scope][https://github.com/weaveworks/scope]


== クラウドネイティブなアプリケーション開発

マイクロサービスらしいアプリケーションとはのセクションでも少し言及しましたが、アプリケーションには大きく分けて以下の3つがありました。

//table[showks-application-list][showKsプロジェクトで開発したアプリケーションとその役割]{
名前            役割
-------------------------------------------------------------
Canvas         ユーザーに払い出される独立したマイクロサービスアプリケーション
Aggregator     PortalにCanvasの状態を表示するためのAPI集約サーバー
Portal         マイクロサービスアプリケーションの一覧画面
//}

//image[application-architecture][showksアプリのアーキテクチャ][scale=0.8]{
//}

アプリケーション開発においてはさまざまな設計思想や考え方がありますが、クラウドネイティブな環境において非常に大切なことは、「イミュータブル」であることです。

インフラ自体もイミュータブルな構成を実現するためにコンテナを利用しているように、アプリケーションにおいても環境に依存するようなコードは徹底的に排除する必要があります。中でも、以下の点についてはインフラに限らず、アプリケーション側でも考慮すべきでしょう。

  * データの永続化
  ** データベース
  ** ユーザーなどが投稿するファイル
  * 環境変数などの活用
  ** 秘匿情報の取扱い（外部サービスのAPIキーやアプリケーションで利用するシークレットなど）
  ** 接続先エンドポイントなどの情報

これらの情報を徹底的にハードコードさせない。ローカルにストアしないようにさせることが、コンテナ化はもちろんですが、スケーラブルなアプリケーションを開発する上では必須です。

また、マイクロサービスの原則をどれだけ守るかの線引も非常に重要です。現実問題、プロダクトによってはあらゆるコンポーネントを疎結合にするということが難しい場合もあります。同様に、The Twelve-Factor Appにどれだけ寄せられるかというのはアプリケーション開発者としての腕の見せ所ではありますが、継続的にアップデートを続けるための環境づくりとして無視することはできない要素です。
