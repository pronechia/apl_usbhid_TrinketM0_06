# apl_usbhid_TrinketM0_06

<<作業中>>

Arduinoの USB/HID を使って文字列を出力しブラウザーと連携して、エアコンのON/OFFと温度調整制御をおこない、結果をロギング＆プロット出力します。ブラウザー／Arduino／エアコン の仕組みは第五回と同様ですが、冷房／暖房の制御が可能です。さらに Arduino で結果をEEPROMにロギングし、後でIDEのシリアルプロッタに出力します。

詳細はQiita参照 https://qiita.com/pronechia/items/b9623c5b8ac65b95edbf

注意）

・Windows用とMac用で、ソースを分離しました。環境に合わせて利用してください。

各自の環境にあわせて確認、修正する箇所があります。ソースコードの上部のコメントを参考に、対応してください。

・実行時の注意

WindowsPC、Macとも、入力モードは、英数字である必要があります。日本語モードになっていないことを確認してください。

This is an Arduino USB/HID with Tone sample program .
