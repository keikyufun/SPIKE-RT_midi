# SPIKE-RT-MIDI Converter

このプロジェクトは、MIDIファイルを解析し、SPIKE RTの `hub_speaker_play_tone` と `hub_speaker_set_volume` 関数に変換するコンバータです。

## バージョン
- **C言語版**: コマンドラインで動作するプログラム
- **HTML版**: Webブラウザで動作するインターフェース
- **Python版**: コマンドライン

## Python版 使用方法
pretty-midiライブラリをベースにしています。

1. Python 3がインストールされていることを確認
2. ./convert_py.shを実行
3. トラックリストが表示される
4. 変換するトラック番号を入力
5. `output.txt` にCコードが生成される

## C版 使用方法
pretty-midiライブラリをベースにしています。

1. Python 3がインストールされていることを確認
2. ./convert_c.shを実行
3. トラックリストが表示される
4. 変換するトラック番号を入力
5. `output.txt` にCコードが生成される

## HTML版 使用方法
1. `index.html` をWebブラウザで開く
2. MIDIファイルをアップロード
3. 「MIDIファイルを読み込み」ボタンをクリック
4. トラックを選択
5. 「トラックをプレビュー」でノート情報を確認
6. 「トラックを再生」でブラウザで音を聴く（Web Audio API使用）
7. 「変換」ボタンでコードをテキストエリアに表示（output.txtにコピーして使用）

## 出力例
生成されるコード例：

```c
// SPIKE RT 用変換コード
#include <spike/hub/speaker.h>

void play_midi() {
    hub_speaker_set_volume(100); // デフォルト音量
    hub_speaker_set_volume(64);
    hub_speaker_play_tone(440, 500);
    // ...
}
```

## 注意
- テンポは120 BPMを仮定（C言語/HTML版）
- つまり音の長さを考慮しません。
- Python版はMIDIファイルのテンポを考慮
- ノートオフの対応は簡易的
- 実際のSPIKE RT環境で使用するには、生成されたコードをプロジェクトに組み込んでください

## コンパイル (C言語版)
```bash
gcc main.c -o midi_converter -lm
```

## 依存 (Python版)
- Python 3
- pretty-midi (クローン済み) / apt: python3-mido, python3-numpy

### シェルスクリプト使い方
1. `chmod +x convert_c.sh convert_py.sh`
2. C版: `./convert_c.sh`
3. Python版: `./convert_py.sh`

### convert options
- C版: `./midi_converter <midi> [track] [--all] [--fast] [--overwrite|--yes]`
- Python版: `python3 midi_to_spike.py <midi> [track] [--all] [--fast] [--overwrite|--yes]`

#### 使用例

##### 1. 実行権限の付与
最初に一度だけ実行してください。
```bash
chmod +x convert.sh
```

###### 2. 実行
スクリプトを起動します。
```bash
./convert_py.sh
```

###### 3. 操作方法
実行すると、カレントディレクトリの `.mid` ファイルがリスト表示されます。

- **個別変換**: 番号を入力し、続けてトラック番号（デフォルト0）を入力します。
- **一括変換**: `all` と入力すると、全ファイルを `--fast --overwrite` モードで変換します。

###### 4. 構成
- `convert.sh`: このシェルスクリプト
- `midi_to_spike.py`: 変換本体のPythonプログラム

C版も同様のオプションで動作します。

### 生成内容
- `<midi名>/track<番号>/output.txt` ー SPIKE RT コード
- `<midi名>/track<番号>/play.html` ー ブラウザ視聴用HTML