= クラウドネイティブを実現する全体アーキテクチャ

== 甘すぎた見積もり

そんなわけで始まったshowKsの開発。

いくらクラウドネイティブだ、マイクロサービスだと言っても、所詮はショーケース。商用サービスでもないので、構成は極めてシンプルなものでいいだろう。そう思っていました。 実際、最初に描いた全体図はこんなイメージだったのです。

//image[nyancat][nyancat]{
//}

プロジェクトスタート遅くなったけど、これなら実現できそうだよね。 でも、僕らのそんな淡い期待は早々に打ち砕かれることになるのです。

== 持ち帰りを実現するキーワード 『Infrastructure as Code』

showKsでは参加者が会社に持ち帰り、クラウドネイティブなCI/CDを実現できることも目的の一つとなっていました。
そのため、様々な環境構築から設定などはいわゆるInfrastructure as Codeとして全てコード化していきました。

=== Terraform

今回のshowKsではGoogle Cloud Platform（GCP）を利用しました。
Terraformでは、Google Kubernetes Engine（GKE）クラスタの構築・DNSの設定・グローバルIPアドレスの確保といった処理を行い、環境の準備を行うようになっています。

https://github.com/containerdaysjp/showks-terraform

=== Kubernetes

Kubernetesでは、YAML形式で書かれたマニフェストを用いて設定を行なっていきます。
そのため、Kubernetesの関する設定は自動的にコード化することが可能でした。

=== Helm

Kubernetesのマニフェストを書いていくと、同じようなYAMLファイルをたくさん書く必要が出てきます。
大量のマイクロサービスが作られる環境では、共通な部分も多いため、HelmをYAMLのテンプレートエンジンとして利用して共通化を行いました。

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

== マイクロサービスらしいアプリのアーキテクチャ、すなわち・・・

== 運用無くして何がクラウドネイティブだ。 モニタリングツールを考えよう

== マイクロサービスの可視化

== クラウドネイティブなアプリケーション開発

https://github.com/containerdaysjp/showks-canvas

実際に完成してから開発効率が格段に向上
