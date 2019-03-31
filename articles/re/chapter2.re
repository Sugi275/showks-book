= クラウドネイティブのショーケースを作る

== 当初の計画

前章で説明した全体コンセプトが決定したのが9月末頃。12月頭のJKD本番まで、あと2ヶ月とすこしというタイミングでした。決して余裕のあるスケジュールではないものの、まだこの段階では楽観視していたのです。なぜなら、やるべき事が明確であり、仕組みもシンプルであるように思えたからです。 このとき、僕らの頭の中にあるイメージは@<img>{at_first}のようでした。

//image[at_first][当初の構想][scale=0.6]{
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

ファイルはGitHub@<fn>{showks-terraform}からダウンロード可能です。

//footnote[showks-terraform][https://github.com/containerdaysjp/showks-terraform]

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

利用したPipeline YAMLはGitHub@<fn>{showks-concourse-pipeline}からダウンロード可能です

(全体的に説明の追加要)

//footnote[showks-concourse-pipeline][https://github.com/containerdaysjp/showks-concourse-pipelines]

=== （モテるから）Continuous Deliveryやりたいよね - Spinnaker

Spinnakerというツールはご存じでしょうか。触ったことはないけど、名前だけは知っているという方も多いのでは無いでしょうか？

よくCI/CDという形でひとまとめに語られますが、Continuous IntegrationとContinuous Deliveryは異なるものであり、それぞれに適したツールが存在します。Spinnakerは、Continuous Deliveryに特化したツールです。CIツールでデリバリーまでやってしまうことも可能なのですが、やはりショーケースとしてこの人気ツールを外すことはできないでしょう。というわけで、ツールがひとつ増えました。

Spinnakerでは TODO からPipeline設定をYAMLで定義することが可能になったため、showKsではWebUIから登録するのではなく、YAMLから登録する形にしました。


SpinnakerのPipeline YAMLはGitHub@<fn>{spinnaker-pipelines}からダウンロード可能です

(全体的に説明の追加要)

//footnote[spinnaker-pipelines][https://github.com/containerdaysjp/showks-spinnaker-pipelines]

//image[iac][CI/CDの実践][scale=0.6]{
//}

== GitOps

GitOpsはWeaveworksが提唱するContinious Deliveryを実現する手法の一つです（https://www.weave.works/technologies/gitops/）。
GitOpsではContinious DeliveryをGitリポジトリを介して行うことにより、全ての変更はPull-Requestをベースに行われていきます。
GitOpsで登場するリポジトリは2種類存在します。一つはアプリケーションのソースコード用のリポジトリ、もう一つはKubernetesのマニフェスト用のリポジトリです。
流れとしては、まずアプリケーション開発者はアプリケーションのリポジトリに対してPull-Requestを作成後、UnitTest・コンテナイメージのビルドなどのContinuous Integrationが行われます。
ここまでは従来のアプリケーション開発と何も変わりません。
GitOpsではこのCIが行われた際に、マニフェスト用のリポジトリに対して更新を行います。より具体的にはマニフェストに記述された利用するコンテナイメージ（spec.containers[].image）のタグ（バージョン）を変更します。
その後リポジトリのマニフェストが更新されたことを検知すると、実際にKubernetesクラスタに対して変更が加えられるようになります。

//image[gitops][GitOpsの実践][scale=0.6]{
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

== ユーザー体験の設計

ツール決めだけで無く、ユーザー体験の面も結構な時間を割いてディスカッションしました。今回のコンセプトのひとつが『クラウドネイティブな開発の体験』なんですから、そこをおざなりにすることは出来ません。 JKDの運営委員会の後にみんなで残って、やんややんやとディスカッションを行ったことを覚えています。

//image[ux-discussion][ユーザー体験についてのディスカッション@インプレス][scale=0.6]{
//}

参加者は、アプリケーションの開発者になった気持ちになってもらい、コードを書いた後GitHubにpushしてもらいます。もちろん、イベント中に一からコードを書くのは難易度が高い話になってしまいますので、代わりの仕組みとしてJSONを編集すると、その情報がキャンバスに反映されるようにしました。

ブランチ戦略としてはstagingとmasterを準備。本当の開発だとfeature-staging-masterと3段階くらいに分かれているケースが多いのではないかと思いますが、今回はイベント中の体験重視で2段構えのブランチとしました。

stagingにpushされたコードは、Concourse CIでユニットテストが行われます。InvalidなJSONであったり、JSON以外の編集があった場合はテストがコケるようになっています。JSON以外のコードを受け付けないようにしているのは、悪意のあるコードをshowKs環境で動かされるのを防ぐためです。

ユニットテストが通過したら、コンテナイメージのbuildとタグ付け、レジストリへのpushを行います。その後、KubernetesのマニフェストをHelmテンプレートから生成し、デプロイします。これでStaging環境で動作確認が行えるようになります。

その後、開発が終わったとの想定でstagingからmasterブランチに対してPull Requestを投げて貰います。Concourse CIがそれを検知し、テストした後問題が無ければLGTMのコメント後、マージを行います。

これでmasterブランチに変更が取り込まれましたので、その変更を再びConcourseが検知し、Stagingで作成したイメージへのタグ追加とマニフェスト生成を行います。最後にSpinnakerがデプロイを行って、Productionリリースが完了となります。フローを図にしたものが、@<img>{developer-experience}です。シンプルさを重要視した結果、GitHub Flow@<fn>{github-flow}と呼ばれるブランチ戦略とほぼ同じ仕組みになりました。

//footnote[github-flow][http://scottchacon.com/2011/08/31/github-flow.html]

//image[developer-experience][デベロッパーワークフロー][scale=0.8]{
//}

!!!! ブランチ分けでやるのか、リポジトリ分けでやるのかの話も書く

== 本番を想定するならば、少なくとも2面は環境必要だよね

ディスカッションしている間に、こんな話も出てきました。『パイプラインをProductionとStagingに分けて作ってるけどさあ、環境自体はどうする･･･?』

確かに僕らは今までProduction,Stagingを区別してコードを書いてきましたが、実際にアプリケーションを載せるk8sのほうは、特に何も考えていなかったのです。でも、本当にProductionに出すとすると、k8sクラスタは分けるケースが多いのではないでしょうか。クラウドネイティブのショーケースを名乗るのであれば、やっぱりここも分けた方がいいかもしれません。

k8sクラスタが分かれるということは、GitOps的にもリポジトリが分かれます@<fn>{showks-manifests-prod}@<fn>{showks-manifests-stg}。そして、それをデプロイするSpinnakerのパイプラインも分かれることになります。図にすると@<img>{separate-env}のようになるでしょうか。 やばい、どこまで構成大きくなるんだろう。

//image[separate-env][ProductionとStagingの分離][scale=0.6]{
//}


//footnote[showks-manifests-prod][https://github.com/containerdaysjp/showks-manifests-prod]
//footnote[showks-manifests-stg][https://github.com/containerdaysjp/showks-manifests-stg]

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

最終的に、このフォームはRuby on Railsで書くことになりました。showks-formという名前で、独立したアプリケーションとして動かすことにします@<fn>{showks-form}。

RoRを採用したことで、ActiveRecordの持つ強力なバリデータによって、たった数行@<list>{project.rb}でバリデーションを行うことができました。

//listnum[project.rb][バリデーション部分][ruby]{
class Project < ApplicationRecord
  include ActiveModel::Validations
  validates_with GitHubUserValidator
  validates :username, uniqueness: true, presence: true, format: { with: /\A[a-z0-9\-]+\z/}, length: { maximum: 30 }
  validates :github_id, uniqueness: true, presence: true, length: { maximum: 30 }
  validates :twitter_id, format: { with: /\A[a-zA-Z0-9\_]+\z/}, length: { maximum: 15 }
  validates :comment, length: { maximum: 100 }
(略)
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
(略)
//}

Concourseは標準のCLIであるflyコマンド、Spinnakerはspinコマンドを、Railsから実行するという形の実装となっています。

これらの処理が、ユーザーの申し込みごとに行われることになります。1ユーザー申し込みごとに、GitHubのリポジトリが1つ、Concourseのパイプラインが3つ、Spinnakerのパイプラインが2つ出来上がります。もしamsy810, jacopen, jyoshiseの3ユーザーが申し込んだとすると、作成されるリソースは@<img>{provisioned-resources}のようになるわけですね。

//image[provisioned-resources][作成されたリソース][scale=0.6]{
//}

===[column] Pipeline as Code

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

<ここ執筆お願いしたいです @amsy810 or jyoshise>

===[column] カナリアリリース

カナリアリリースは、炭鉱夫が有害な一酸化炭素や有毒ガスを検知するためにカナリアを鳥かごに入れて連れていったことに由来すると言われています。
カナリアは人間よりも有毒ガスなどに敏感なため、人間よりも早く検知することができるため、早く逃げることができるようになります。
カナリアリリースは一部のユーザに対してリリースを行い問題にいち早く気づいてもらうためのリリース手法といえば、イメージも掴みやすいのではないでしょうか。

また、従来は十分にテストをしてからリリースする傾向でしたが、近年マイクロサービス化を含むクラウドネイティブ化された状況ではカナリアリリースなどを利用してリスクを低減することで、本番環境へのリリースを気軽に行えるようにもなってきたなと感じています。
リリースが早まることで、新しい機能をいち早くユーザに届けることができるのもクラウドネイティブがなし得ることですね。

===[/column]


== マイクロサービスらしいアプリケーションとは

マイクロサービスマイクロサービスと世間では囃し立てられますが、そもそもマイクロサービスの定義は何なのでしょうか。

マイクロサービスは、「サービス」という単位でビジネスロジックや各機能を疎結合な形で複数に分割し、それらを組み合わせることで大きなアプリケーションを作り上げる開発手法です。各サービスの独立性が求められるため、Dockerコンテナを使ってあげることで論理的な要素の分離と素早いサイクルでの更新に耐えうるシステムが構築できます。また、複数の別の機能を持つサービスが同時に走るような構成を作り上げる上で、Kubernetesという分散システムを使った基盤は非常にその特性と相性がよいとされており、「マイクロサービス」と「Kubernetes」はしばしば同じ文脈で用いられます。

プロジェクトをはじめたきっかけとして「クラウドネイティブなインフラを作って、参加者の人に面白さを伝える」という思いが僕たちにはありました。そのため、インフラの構想はどんどん膨らむ一方、アプリケーションの中身やその設計についてはあまり深く考えられていませんでした（正確に言うと、アプリケーションの実装を実際に行う人が少なかったとも言えます）。そして、あれこれ手を動かしながら試行錯誤を繰り返す中で、次第に以下のような形に収束していきました。

 * 参加者は1つの独立したサービスを持ち、それを自分で管理できる
 * 各マイクロサービスは参加者のアクション（showKs form）を起点として動的に払い出される
 * 各サービスは共通のエンドポイントを持ち、それをアグリゲーターによって集約して可観測な状態にする
 * アグリゲーターはKubernetesのAPIを使って後ろにいる各サービスたちを常に監視していて、サービスが増えたり減ったりしたときにはそれがリアルタイムで更新される
 * クライアントに提供されるアプリはアグリゲーターとだけ通信を行い、得られた結果をクライアントに伝える

こうして、ユーザーに提供するためのサービスである「canvas」、通信を集約する「aggregator」、そして、各サービスのステータスを可視化する「portal」の3つのアプリを作成することになりました。

== 運用無くして何がクラウドネイティブだ。 モニタリングツールを考えよう

== マイクロサービスの可視化

== クラウドネイティブなアプリケーション開発

マイクロサービスらしいアプリケーションとはのセクションでも少し言及しましたが、アプリケーションには大きく分けて以下の3つがありました。

//table[showks-application-list][showKsプロジェクトで開発したアプリケーションとその役割]{
名前            役割
-------------------------------------------------------------
Canvas         ユーザーに払い出される独立したマイクロサービスアプリケーション
Aggregator     PortalにCanvasの状態を表示するためのAPI集約サーバー
Portal         マイクロサービスアプリケーションの一覧画面
//}

https://github.com/containerdaysjp/showks-canvas

実際に完成してから開発効率が格段に向上
