# convert3D
3Dモデルの形式変換をするツールです

windowsシェルエクステンションを利用しているため、コンテクストメニュー(右クリックメニュー)に登録されます
* Windows 11の場合は「その他のオプションを表示」の中に登録されます ([参考](https://forest.watch.impress.co.jp/docs/serial/win11must/1363937.html))

## インストール方法
1. [Release](https://github.com/n-taka/convert3D/releases/tag/v1.0)からzipアーカイブをダウンロード
2. zipアーカイブを適当な場所に展開
3. フォルダの中に入っている"install.bat"を**管理者として**実行

## アンインストール方法
1. フォルダの中に入っている"uninstall.bat"を**管理者として**実行
* convert3D.dllを削除するだけでも、見かけ上はアンインストール出来ます。しかし、レジストリの一貫性が保たれなくなるため非推奨です

## 使い方
まとめたいファイルを選択後、右クリックメニューから"convert3D > 変換先フォーマット"を選択してください

- 変換*元* 対応フォーマット
  - OBJ
  - PLY
  - STL
  - OFF
  - WRL
  - MSH
  - MESH

- 変換*先* 対応フォーマット
  - OBJ
  - PLY
  - STL
  - WRL
  - OFF
  - MESH

- 現時点では、形状情報のみ対応しています
  - 頂点カラーやテクスチャに関しては必要になったら対応します

## 注意事項
* Windows 10の場合、アンインストール直後にconvert3D.dllが削除できません (Windows explorerによって利用中というエラーが出ます)
  * 次回再起動以降、削除できるようになります
  * 再起動せずに、タスクマネージャー等を利用してWindows explorerを再起動しても削除できるようになります
  * Windows 11の場合は上記トラブルは発生しないようです

## 動作確認
* Windows 11 Pro (23H2)

## その他
[Microsoftのコードサンプル](https://github.com/microsoftarchive/msdn-code-gallery-microsoft/tree/master/OneCodeTeam/C%2B%2B%20Windows%20Shell%20context%20menu%20handler%20(CppShellExtContextMenuHandler))をベースにプログラムの作成を行いました。

## 連絡先
[@kazutaka_nakash](https://twitter.com/kazutaka_nakash)

## ライセンス
[MITライセンス](https://github.com/n-taka/convert3D/blob/main/LICENSE)
