# NTcounter

赤外線の送信機・受信機のペアで、その間を横切った数をカウントするシステムです。NT金沢のような開放空間での来場者カウントに使います。横切りがあった時刻と積算カウント数をmicroSDカードに記録していきます。

# 用意するもの
## 送信機
- [ATtiny10](http://akizukidenshi.com/catalog/g/gI-04575/)
- [nMOS](http://akizukidenshi.com/catalog/g/gI-06051/)
- [赤外線LED](http://akizukidenshi.com/catalog/g/gI-12612/)
- USB Type-Cコネクタ (a)http://www.aitendo.com/product/17279 または (b)http://akizukidenshi.com/catalog/g/gC-14356/
- チップ抵抗 5.1kΩ(1608)×2個、1kΩ(1608)×1個
- チップLED 緑(1608)×1個
- プリント基板 NTcounterTX
- [100均ルーペNo.13058]（セリア等で売っているようです）
<img src="https://github.com/akita11/NTcounter/blob/master/NTcounerTX/loupe.jpg" width="240px">
- 厚紙
- MDF板(厚さ6mm) ※アクリル板(厚さ5mm)でもOK
- USBモバイルバッテリ等の電源と、それをつなぐUSB Type-C等のケーブル
- M3x12ネジ+M3ナット×2組

## 受信機
- [M5Stack Basic](https://www.switch-science.com/catalog/3647/)
- [M5Stackバッテリモジュール](https://www.switch-science.com/catalog/3653/)
- [M5Stackコネクタ](https://www.switch-science.com/catalog/3654/)
- [赤外線リモコン受信機](http://akizukidenshi.com/catalog/g/gI-04659/)
- [スライドスイッチ](http://akizukidenshi.com/catalog/g/gP-08789/)
- [ピンソケット1x3p](http://akizukidenshi.com/catalog/g/gC-10098/)
- プリント基板 NTcounterRX
- microSDカード
- アクリル板(厚さ5mm)

※おまけ：プリント基板NTcounerRXは、M5StackのGroveコネクタのポートA, B, C, D, Eの拡張にも使えます

# 作り方

## 送信機

1. プリント基板NTcounterTXに部品をとりつけます。USB Type-Cコネクタとして(a)を用いる場合はCN4に、(b)を使う場合はCN2にとりつけます。なおCN4につける(a)の方がはんだ付けはしやすいです。
2. 、ファームウエアNTcounterTX.hexを書き込みます。(NTcounterTX.inoをArduinoIDEからコンパイルして書き込んでもOK。（参考: [ArduinoIDEでATtiny10を開発・書き込み ](https://make.kosakalab.com/make/electronic-work/arduino-ide/arduino_tpi/)
3. MDF板(厚さ6mm)にNTcounerTX_lens.svgの右側の部品をレーザーカッター等でカットし、1.を固定します。
<img src="https://github.com/akita11/NTcounter/blob/master/NTcounterTX1.jpg" width="320px">
4. 厚紙にNTcounerTX_lens.svgの左側の部品をレーザーカッター等でカットします。このとき、用いるUSB-Cコネクタが(a)の場合と(b)の場合で少し加工データが異なるので、NTcounerTX_lens.svgを適宜修正してください。
<img src="https://github.com/akita11/NTcounter/blob/master/NTcounterTX2.jpg" width="320px">
5. 100均ルーペの3倍を、取っ手を外して、1.とともに、2.で囲って固定します。
<img src="https://github.com/akita11/NTcounter/blob/master/NTcounterTX3.jpg" width="240px">

## 受信機

1. プリント基板NTcounterRXに部品をとりつけます。（※M5Stackコネクタのオス・メスを取り付ける面を間違えないように注意。M5Stack本体(M5Core)に差し込んだ状態でシルク文字が外から見える向き）
2. VSCodeでNTcounerRX内のファームウエアの以下の箇所を自分の環境に合わせて修正します。
  - #define DEV_NAME "NTcounter" ← デバイス名。ログファイルの冒頭に記録される
  - const char* ssid       = "****"; ← 起動時に接続するWiFiアクセスポイントのSSID
  - const char* password   = "****"; ← 同パスワード
3. ファームウエアをビルドして書き込みます。
4. アクリル板(厚さ5mm)をNTcounterRX_spaccer.svgのようにレーザーカッター等でカットします。
5. M5StackのCore、1.とスペーサ、M5Stackバッテリユニット、、M5Stack底板を順にとりつけます。
<img src="https://github.com/akita11/NTcounter/blob/master/NTcounterRX1.jpg" width="320px">

# 使い方
1. USBモバイルバッテリ等からUSB TypeｰCケーブルで送信機に給電します。電源ランプが点灯します。
2. 受信機を充電後、microSDカードをとりつけて電源スイッチをONにします。なお受信機の起動時のみ、時刻あわせのためWiFi接続が必要です。起動後はWiFiは不要です。
3. 送信機と受信機を向かい合わせに、通路をはさむ両側に置ます。このとき、送信機の赤外光が受信機に届いていれば、受信機のディスプレイに表示される円が緑になります。円が赤い場合は、赤外光が届いていないので、向きを調整します。両者の間隔は1〜6mくらいまでは大丈夫なはずです。
4. この状態で、両者の間に障害物を置くと、受信機に表示される円が赤になり、その時点の時刻と積算カウント数が記録されます。
5. 動作がOKそうなら、受信機のM5StackのBボタン（中央ボタン）を押し、画面表示をOFFにすると、バッテリが長持ちします。（満充電で丸2日くらいはもつはず）

# Author

@akita11 (akita@ifdl.jp)

