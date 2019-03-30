= クラウドネイティブを実現する全体アーキテクチャ

== 全体アーキテクチャ

今回構築したshowKsのアプリケーションは、以下のようなアーキテクチャで構成されています。

//indepimage[01/pre7][フォーム][scale=0.5]

== 甘すぎた見積もり

そんなわけで始まったshowKsの開発。

いくらクラウドネイティブだ、マイクロサービスだと言っても、所詮はショーケース。商用サービスでもないので、構成は極めてシンプルなものでいいだろう。そう思っていました。 実際、最初に描いた全体図はこんなイメージだったのです。

//image[nyancat][nyancat]{
//}

プロジェクトスタート遅くなったけど、これなら実現できそうだよねと。 でも、僕らのそんな淡い期待は早々に打ち砕かれることになるのです。なぜなら、実際に作り始めると、あれもこれも必要だと言うことが分かってきたのです。それでは、どういうツールをどういう考えで使ったのか、紹介していきます。

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

アプリケーションはすべてKubernetesの上で動かします。KubernetesはYAML形式で書かれたマニフェストを用いて設定を行ってく仕組みのため、何もしなくても自動的にIaCが実現できることになります。

=== Helm

Kubernetesのマニフェストを書いていくと、同じようなYAMLファイルをたくさん書く必要が出てきます。しかし、大量のマイクロサービスが作られる環境では共通する部分も多く、上手くテンプレート化することで記述量を削減することができます。

マニフェスト作成を助けるツールはいくつかありますが、今回はその中でも最も有名なHelmを採用しました。Helmはパッケージマネージャーとして知られており、 `helm install` コマンドで様々なアプリケーションを簡単にKubernetesにデプロイすることが出来ます。 

しかし、今回はパッケージマネジメントの仕組みは使わず、純粋なテンプレートエンジンとして利用しています。

(helmの図解やもうちょっと細かい説明をここに入れたい)

https://github.com/containerdaysjp/showks-canvas/tree/master/helm

=== Concourse

今回はCIツールとしてConcourseCIを利用しました。
ConcourseCIではCI PipelineをYAMLで定義することが可能なため、こちらもコード化を行うことが可能です。

https://github.com/containerdaysjp/showks-concourse-pipelines

=== Spinnaker

また、CDツールにはSpinnakerを利用しました。
Spinnakerでは TODO からPipeline設定をYAMLで定義することが可能になったため、showKsではWebUIから登録するのではなく、YAMLから登録する形にしました。

https://github.com/containerdaysjp/showks-spinnaker-pipelines

== ユーザーに見せるだけではなく、何らかの形で参加して欲しいアプリ

== （モテるから）Continuous Deliveryやりたいよね

Spinnakerというツールはご存じでしょうか。触ったことはないけど、名前だけは知っているという方も多いのでは無いでしょうか？

よくCI/CDという形でひとまとめに語られますが、Continuous IntegrationとContinuous Deliveryは異なるものであり、それぞれに適したツールが存在します。Spinnakerは、Continuous Deliveryに特化したツールです。CIツールでデリバリーまでやってしまうことも可能なのですが、やはりショーケースとしてこの人気ツールを外すことはできないでしょう。 というわけで、ツールがひとつ増えました。

<図>

== GitOps

== 申し込みフォームいるじゃん！どうしよう

== 爆誕　Pipeline as a Code

== 本番を想定するならば、少なくとも2面は環境必要だよね
https://github.com/containerdaysjp/showks-manifests-prod
https://github.com/containerdaysjp/showks-manifests-stg

== カナリアリリース

ingress-nginx ギリギリリリースされる

== マイクロサービスらしいアプリケーションとは

マイクロサービスマイクロサービスと世間では囃し立てられますが、そもそもマイクロサービスの定義は何なのでしょうか。

今回、僕たちのチームは「クラウドネイティブなインフラを作って、参加者の人に面白さを伝える」という話をきっかけにプロジェクトをスタートさせました。そのため、インフラの構想はどんどん膨らむ一方で、アプリケーションの中身やその設計について深く考えることをしませんでした（正確に言うと、アプリケーションの実装を実際に行う人が少なかったとも言えます）。

マイクロサービスは、「サービス」という単位でビジネスロジックや各機能を疎結合な形で分割し、それを複数組み合わせることで大きなアプリケーションを作り上げる開発手法です。複数の別の機能を持つサービスが同時に走るような構成を作り上げる上で、Dockerコンテナを使った基盤は非常に相性が良く、Kubernetesという分散システムも相まって「マイクロサービス」と「Kubernetes」はしばしば同じ文脈で用いられます。

今回最終的にアプリケーションとして作ったのは以下の構成になりました

* 参加者が1つのマイクロサービスを持つ

* 各サービスはAPIのアグリゲーターを経由してクライアントと通信を行う(BFF)

* クライアントアプリはバックエンドに待ち受けるアグリゲーターと通信を行い、結果をクライアントに伝える(Server Side Rendering)

この結果、各マイクロサービスは参加者のアクションを起点として動的に払い出され、アグリゲーターがKubernetesnoAPIを使って通信を拾い上げてAPIの終端となるような構成になりました。

== 運用無くして何がクラウドネイティブだ。 モニタリングツールを考えよう

== マイクロサービスの可視化

== クラウドネイティブなアプリケーション開発

https://github.com/containerdaysjp/showks-canvas

実際に完成してから開発効率が格段に向上
