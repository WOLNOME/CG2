＜一日一機能！＞

<追加する機能>

<今回の制作の煩わしかった点>

<追加する機能>

<スケジュール>
https://brabioproject.jp/group/ahJzfmJyYWJpb3Byb2plY3RocmRyDwsSBUdyb3VwGIPG0MJEDA/

##GPUパーティクル、次の段階へ
・Particleクラスに「ParticleCSCommon.hlsli」に対応するような構造体を作成。
今のところ必要なリソースは、「粒情報」「フリーリスト」「フリーリストのインデックス」「エミッター」「Json情報」「時間情報」の6つ。
「粒情報」: SRV,UAVの両方ビューを作る。→UAVはCSで、SRVはVSで使うため。(VSで使うため、バリア遷移は必須。また、ルートパラメーターの設定も必須。)
「フリーリスト」: UAVのビューを作る。
「フリーリストのインデックス」: UAVのビューを作る。
「エミッター」: CBVで作る。CSで使う。
「JSON情報」: CBVで作る。CSで使う。
「時間情報」: CBVで作る。CSで使う。
・CS用コンピュートルートシグネチャ、コンピュートパイプラインのセットをinit,emit,updateの3つ作る。→Manager側で。
・VSを「粒情報」に対応した形で作り直す。(lifeTimeが0なら棄却する感じ。)
・ParticleManagerで各Particle分emitとupdateを回す。initはParticleクラスのInitializeで最後の方に回せばよさそう

＜最終的にParticleCreatorシーンで正しく動けば完成＞
エディターにしっかり対応させるように作り直すのも忘れずに。