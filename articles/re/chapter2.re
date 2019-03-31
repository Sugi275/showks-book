= クラウドネイティブのショーケースを作る

== 当初の計画

前章で説明した全体コンセプトが決定したのが9月末頃。12月頭のJKD本番まで、あと2ヶ月とすこしというタイミングでした。決して余裕のあるスケジュールではないものの、まだこの段階では楽観視していたのです。なぜなら、やるべき事が明確であり、仕組みもシンプルであるように思えたからです。 このとき、僕らの頭の中にあるイメージは@<img>{at_first}のようでした。

//image[at_first][当初の構想][scale=0.5]{
//}

シンプル。実にシンプル。これなら1ヶ月もあれば実現できそうです。

でも、僕らのそんな淡い期待は早々に打ち砕かれることになるのです。なぜなら、実際に作り始めると、あれもこれも必要だと言うことが分かってきたのです。それでは、どういうツールをどういう考えで使ったのか、紹介していきます。


== 持ち帰りを実現するキーワード 『Infrastructure as Code』

第1章でも述べたように、showKsでは@<b>{お持ち帰り}が大きなテーマのひとつでした。お絵描きアプリで遊んで、その仕組みを説明されただけでは、なかなか知識として定着しませんよね。僕たちが作った仕組みそのものを持ち帰ってもらい、皆さんの環境上でクラウドネイティブなCI/CDを実現してもらいたかったのです。

だからといって、昔ながらの@<b>{構築手順書}には絶対にしたくなかったんです。だって@<b>{かっこ悪いもん}。じゃあどうするか？ コード化するに決まってるでしょう！ そう、Infrastructure as Code(IaC)をやれば、持ち帰りやすくなりますよね。

=== Terraform

マネージドのKubernetesを使いたい！となったとき、第一の選択肢に上がるのがGoogle Kubernetes Engine(GKE)でしょう。今回のshowKsでも、GKEを利用しました。その他、アプリケーションで利用するファイルの置き場や静的IPなども必要となりますので、以下のサービスを利用しました。

 * Google Cloud Storage
 * Google Cloud DNS
 * Static External IP
 * Google Compute Engine(踏み台サーバー用)

マネージドサービスはとても便利ですし、皆さん結構GUIを使ってポチポチ構築されているのではないでしょうか。でも、GUIを使うのは手数がかかりますし、再現性の点においてもあまり好ましくありません。そこで、これらをプロビジョニングする仕組みとして、HashiCorpのTerraformを利用しました。

ファイルは(https://github.com/containerdaysjp/showks-terraform) からダウンロード可能です。

=== Kubernetes

アプリケーションはすべてKubernetesの上で動かします。KubernetesはYAML形式で書かれた宣言的コード（マニフェスト）を用いて設定を行っていく仕組みのため、何もしなくても自動的にInfrastructure as Codeが実現できることになります。
例えば、nginxのコンテナを3つ起動する場合は下記のようなマニフェスト記述し、Kubernetesに登録するだけでその状態に維持し続けてくれます。
そのため、Kubernetesでアプリケーションのアップデートを行う際には、下記のマニフェストの17行目の「nginx:1.12」を「nginx:1.13」のようにイメージタグを変更して再登録することで利用するコンテナイメージ（アプリケーション）のアップデートを行うことになります。

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

Kubernetesのマニフェストを書いていくと、同じようなYAMLファイルをたくさん書く必要が出てきます。しかし、大量のマイクロサービスが作られる環境では共通する部分も多く、上手くテンプレート化することで記述量を削減することができます。
マニフェスト作成を助けるツールはいくつかありますが、今回はその中でも最も有名なHelmを採用しました。Helmはパッケージマネージャーとして知られており、 `helm install` コマンドで様々なアプリケーションを簡単にKubernetesにデプロイすることが出来ます。
しかし、今回はパッケージマネジメントの仕組みは使わず、純粋なテンプレートエンジンとして利用しています。

例えば通常のWebアプリケーションを動作させる場合、コンテナを起動させるDeploymentリソースとServiceリソースを同時に作成することが多いかと思います。
こういった際にはマイクロサービスごとに同じような大量のマニフェストを定義しなければならず、変更漏れなどの可能性も出てきてしまいます。
Helmでは下記のようなテンプレートとValuesファイルをを利用することで、マニフェストのテンプレーティングを行うことが可能です。

//listnum[Helmテンプレートの例][yaml]{
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

//listnum[valuesファイルの例][yaml]{
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



(helmの図解やもうちょっと細かい説明をここに入れたい)


== CI/CD

=== Concourse

今回はCIツールとしてConcourseCIを利用しました。
ConcourseCIではCI PipelineをYAMLで定義することが可能なため、こちらもコード化を行うことが可能です。

https://github.com/containerdaysjp/showks-concourse-pipelines

(全体的に説明の追加要)

=== （モテるから）Continuous Deliveryやりたいよね - Spinnaker

Spinnakerというツールはご存じでしょうか。触ったことはないけど、名前だけは知っているという方も多いのでは無いでしょうか？

よくCI/CDという形でひとまとめに語られますが、Continuous IntegrationとContinuous Deliveryは異なるものであり、それぞれに適したツールが存在します。Spinnakerは、Continuous Deliveryに特化したツールです。CIツールでデリバリーまでやってしまうことも可能なのですが、やはりショーケースとしてこの人気ツールを外すことはできないでしょう。というわけで、ツールがひとつ増えました。

Spinnakerでは TODO からPipeline設定をYAMLで定義することが可能になったため、showKsではWebUIから登録するのではなく、YAMLから登録する形にしました。


https://github.com/containerdaysjp/showks-spinnaker-pipelines

(全体的に説明の追加要)

//image[iac][CI/CDの実践][scale=0.5]{
//}

== GitOps

GitOpsはWeaveworksが提唱するContinious Deliveryを実現する手法の一つです（https://www.weave.works/technologies/gitops/）。
GitOpsではContinious DeliveryをGitリポジトリを介して行うことにより、全ての変更はPull-Requestをベースに行われていきます。
GitOpsで登場するリポジトリは2種類存在します。一つはアプリケーションのソースコード用のリポジトリ、もう一つはKubernetesのマニフェスト用のリポジトリです。
流れとしては、まずアプリケーション開発者はアプリケーションのリポジトリに対してPull-Requestを作成後、UnitTest・コンテナイメージのビルドなどのContinuous Integrationが行われます。
ここまでは従来のアプリケーション開発と何も変わりません。
GitOpsではこのCIが行われた際に、マニフェスト用のリポジトリに対して更新を行います。より具体的にはマニフェストに記述された利用するコンテナイメージ（spec.containers[].image）のタグ（バージョン）を変更します。
その後リポジトリのマニフェストが更新されたことを検知すると、実際にKubernetesクラスタに対して変更が加えられるようになります。

//image[gitops][GitOpsの実践][scale=0.5]{
//}

GitOpsはあくまでも概念であり、マニフェストの更新や更新時のKubernetesクラスタへの適用といった実装は多岐に渡ります。
GitOpsの実装としては、Jenkins X@<fn>{jenkinsx}やWeave Flux@<fn>{flux}などがありますが、今回はHelmを利用したマニフェスト生成とSpinnakerを利用しています。

//footnote[jenkinsx][https://github.com/jenkins-x]
//footnote[flux][https://github.com/weaveworks/flux]

GitOpsを採用することにより、多くのメリットがあります。
1つはKubernetesのマニフェストをGitリポジトリに保管しておくことにより、Kubernetesの状態に関してSingle Source of Truthを実現することが可能です。
システム全体の状態がマニフェストとしてGitリポジトリ上に管理されているため、障害時には再度宣言的に記述されたマニフェストを適用することにより迅速に回復させることが可能になります。
また、Kubernetesへの変更は全てGitリポジトリを介して行われるという特性上、変更の監査履歴としても利用することが可能です。

他にもアプリケーション開発者からみたときに、Kubernetesを意識する必要がほとんど無くなります。
開発者はあくまでもアプリケーションのGitリポジトリに対して変更を加えればよく、そのあとのコンテナをKubernetesへデリバリするのは自動化されたGitOpsの役割になります。
そのため、開発者の生産性を損ねることなく、コンテナの軽量さを生かして生産性やアプリケーションのリリース頻度を高めることが可能です。


===[column] 膨らむ構成

シンプルだなんて言ってたけど、だんだん雲行きが怪しくなってきましたね。この段階で、当初の計画よりも2倍くらい要素が増えてしまいました。アプリケーションだけならまだシンプルだと言えますが、それを上手く運用していくためのツールや手法がとにかく多い。 とはいえ、これらツールの支援無しにはクラウドネイティブな開発が出来ないのも事実。このあたりのツラミについては、第3章にて詳しく説明します。

一方でこのCI/CDの仕組みを一度完成させると、Cloud Nativeの定義で紹介した下記のような利点を享受することができます。
組織の開発力を向上させるためにも、プロジェクトを始める前にしっかりとしたCI/CD環境を整備するのは大切なことなのです。

* 疎結合なシステム
* 復元力がある
* 管理しやすい
* 堅牢な自動化により、頻繁かつ期待通りに最小限の労力で大きな変更が可能

===[/column]

== 申し込みフォームいるじゃん！どうしよう

ここにきて大きな考慮漏れに気づいてしまいました。いや、あえて思考を後回しにしていたとも言えるのですが･･･ 参加型企画なのだから、@<b>{サインアップの仕組み}が必須なのです。何らしかの方法で、参加者から必要な情報を渡して貰う必要があります。

今回、参加者にはGitHubを使ったPull Request開発を体験してもらいますので、GitHubアカウントは必須になります。また、キャンバスに掲載したい情報も渡して貰いたいですね。つまり、以下のような項目の入力が必要なわけです。

 * ユーザー名(必須)
 * GitHubアカウント名(必須)
 * TwitterID (オプション)
 * コメント(オプション)

 当初は、Google Formsを使ってさっくりと用意しようと考えていました。しかし、よくよく考えてみると以下のような制約があることに気づきます。

 * ユーザー名はKubernetesのリソース名に使われるため、ユニークでなくてはならない
 * KubernetesのServiceとしてもユーザー名が使われる。名前解決にも利用されるため、利用できる文字が限られる(例えばアンダースコアはNG)
 * GitHubにもInvitationを送る必要があるため、ValidなGitHubアカウントでなくてはならない

 つまり、ちゃんとしたバリデーションが必要ということになります。ここまでのバリデーションは、Google Formsでやるのは難しそうです。

 最終的に、このフォームはRuby on Railsで書くことになりました。ActiveRecordの持つバリデータによって、たった数行@<list>{project.rb}でバリデーションを行うことができました。

//listnum[project.rb][バリデーション部分][ruby]{

class Project < ApplicationRecord
  include ActiveModel::Validations
  validates_with GitHubUserValidator
  validates :username, uniqueness: true, presence: true, format: { with: /\A[a-z0-9\-]+\z/}, length: { maximum: 30 }
  validates :github_id, uniqueness: true, presence: true, length: { maximum: 30 } #FIXME: need to check validation rule about github id
  validates :twitter_id, format: { with: /\A[a-zA-Z0-9\_]+\z/}, length: { maximum: 15 }
  validates :comment, length: { maximum: 100 }
(略)

//}

=== 爆誕　Pipeline as a Code

申し込みフォームも目処がついたところで、いよいよユーザー申し込みからキャンバスの構築まで具体的なイメージが沸くようになってきました。

一連の流れを図に起こしてみると以下のようになります。

<図>

 * GitHubのリポジトリ作成
 * Concourse
 * Spinnaker

== 本番を想定するならば、少なくとも2面は環境必要だよね
https://github.com/containerdaysjp/showks-manifests-prod
https://github.com/containerdaysjp/showks-manifests-stg

== カナリアリリース

ingress-nginx ギリギリリリースされる

== マイクロサービスらしいアプリケーションとは

マイクロサービスマイクロサービスと世間では囃し立てられますが、そもそもマイクロサービスの定義は何なのでしょうか。

マイクロサービスは、「サービス」という単位でビジネスロジックや各機能を疎結合な形で複数に分割し、それらを組み合わせることで大きなアプリケーションを作り上げる開発手法です。各サービスの独立性が求められるため、Dockerコンテナを使ってあげることで論理的な要素の分離と素早いサイクルでの更新に耐えうるシステムが構築できます。また、複数の別の機能を持つサービスが同時に走るような構成を作り上げる上で、Kubernetesという分散システムを使った基盤は非常にその特性と相性がよいとされており、「マイクロサービス」と「Kubernetes」はしばしば同じ文脈で用いられます。

そもそも僕たちは「クラウドネイティブなインフラを作って、参加者の人に面白さを伝える」という話をきっかけにプロジェクトをスタートさせました。そのため、インフラの構想はどんどん膨らむ一方で、アプリケーションの中身やその設計についてはあまり深く考えられていませんでした（正確に言うと、アプリケーションの実装を実際に行う人が少なかったとも言えます）。あれこれ手を動かしながら試行錯誤を繰り返す中で、次第に以下のような形に収束していきました。

 * 参加者は1つの独立したサービスを持ち、それを自分で管理できる。
 * 各マイクロサービスは参加者のアクション（showKs form）を起点として動的に払い出される。
 * 各サービスは共通のエンドポイントを持ち、それをアグリゲーターによって集約して可観測な状態にする。
 * アグリゲーターはKubernetesのAPIを使って後ろにいる各サービスたちを常に監視していて、サービスが増えたり減ったりしたときにはそれがリアルタイムで更新される。
 * クライアントに提供されるアプリはアグリゲーターとだけ通信を行い、得られた結果をクライアントに伝える。

== 運用無くして何がクラウドネイティブだ。 モニタリングツールを考えよう

== マイクロサービスの可視化

== クラウドネイティブなアプリケーション開発

https://github.com/containerdaysjp/showks-canvas

実際に完成してから開発効率が格段に向上
