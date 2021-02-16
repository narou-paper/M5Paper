# narou-paper
[小説家になろう](https://syosetu.com/)の作品を[M5Paper](https://m5stack.com/products/m5paper-esp32-development-kit-960x540-4-7-eink-display-235-ppi?variant=37595977908396)で読めるように作成したプログラムです。

M5Stack公式が公開している[M5Paper_FactoryTest](https://github.com/m5stack/M5Paper_FactoryTest)をベースに作成しています。

# 使い方
## なろうを読む
1. ホーム画面の「なろうを読む」をタップ
2. 任意の作品をクリックし、任意の話を選択します
## データを入れる
1. [Androidアプリ](https://github.com/narou-paper/flutter-app)を入れる
2. データを入れるをタップし、受信画面にする
3. 受信画面が表示されていることを確認し、Android側からsubmitを押す
# 書き込み手順
1. [VSCode](https://azure.microsoft.com/ja-jp/products/visual-studio-code/)をインストールします
2. [PlatformIO](https://platformio.org/)という拡張をいれます
3. リポジトリをVSCodeで開き、PlatformIOの設定をする
4. 「→」アイコンもしくはPlatformIOのタブ上にある「Upload」をクリックします
5. `data`ディレクトリを作成し、適切なリソースをいれます。リソースのリストは以下に記します(Webのリソースは[こちらのソースコード](https://github.com/narou-paper/M5Paper-Web)をバンドルしてください)
6. PlatformIOのタブ上にある「Upload Filesystem Image」をクリックします

| ファイル名   | 利用用途           |
|:-----------|:-----------------|
| app.js.gz            | Webページのリソース |
| chunk-vendors.css.gz | Webページのリソース |
| chunk-vendors.js.gz  | Webページのリソース |
| favicon.ico.gz       | Webページのリソース |
| index.html.gz        | Webページのリソース |
| 1.png    | ホーム画面のリソース |
| 2.png    | ホーム画面のリソース |
| 3.png    | ホーム画面のリソース |
| 4.png    | ホーム画面のリソース |
| font.ttf    | 各所で使うフォント |
