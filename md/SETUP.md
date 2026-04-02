# セットアップガイド

このガイドでは、SPIKE-RT-MIDI Converterを初期設定する方法を説明します。

## 前提条件

### Web版を使用する場合
- 最新のWebブラウザ
- Python 3.x またはNode.js

### Python版を使用する場合
- Python 3.6以上
- pretty-midi ライブラリ
- numpy

### C言語版を使用する場合
- Cコンパイラ(gcc、clang等)
- Linux/Mac/Windows(WSL)環境

---

## クイックスタート

### ステップ1: ファイル確認

```bash
# プロジェクトディレクトリに移動
cd spike-rt-midi

# ファイル一覧を確認
ls -la
```

以下のファイルが存在することを確認：
- `convert.html` - メインWebインターフェース
- `midi_to_spike.py` - Python変換スクリプト
- `convert_py.sh` - Python用シェルスクリプト
- `main.c` - C言語版ソース
- `convert_c.sh` - C言語版シェルスクリプト
- `DREAM.mid` - サンプルMIDIファイル

### ステップ2: Web版を開く

#### 方法A: ブラウザで直接開く

```bash
# Macの場合
open convert.html

# Linuxの場合
xdg-open convert.html

# またはファイルマネージャーで convert.html をダブルクリック
```

#### 方法B: ローカルサーバー経由(推奨)

**Python 3:**
```bash
python3 -m http.server 8000
```

**Node.js:**
```bash
npx http-server
```

ブラウザで開く:
```
http://localhost:8000/convert.html
```

### ステップ3: Python版をセットアップ(オプション)

```bash
# pretty-midiをインストール
pip install pretty-midi

# テスト実行
python3 midi_to_spike.py DREAM.mid --fast
```

### ステップ4: C言語版をセットアップ(オプション)

```bash
# コンパイル
gcc -o main main.c -lm

# テスト実行
./main DREAM.mid --fast
```

---

## 詳細セットアップ

### ファイル構成

```
spike-rt-midi/
├── convert.html              Webインターフェース
├── midi_to_spike.py         Python変換スクリプト
├── convert_py.sh            Python用シェルスクリプト
├── main.c                   C言語版ソース
├── convert_c.sh             C言語版シェルスクリプト
├── main                     コンパイル済みC実行ファイル
├── README.md                メインドキュメント(日本語)
├── README_EN.md             English Documentation
├── SETUP.md                 このファイル(日本語)
├── SETUP_EN.md              English Setup Guide
├── WEB_CONVERTER_README.md  Web版ガイド(日本語)
├── WEB_CONVERTER_README_EN.md Web版ガイド(英語)
├── TEMPO_CUSTOMIZATION.md   テンポカスタマイズ(日本語)
├── TEMPO_CUSTOMIZATION_EN.md テンポカスタマイズ(英語)
├── DREAM.mid                サンプルMIDIファイル
├── example.mid              別のサンプルMIDIファイル
├── pretty-midi/             Pythonライブラリ
└── examples/                ドキュメント内サンプル
```

---

## 各バージョンの使用方法

### Web版(推奨・初心者向け)

#### 基本的な流れ

1. ブラウザで `convert.html` を開く
2. 「MIDIファイルを読み込む」ボタンでMIDIファイルを選択
3. トラックをドロップダウンから選択
4. 「テンポ(BPM)」フィールドにカスタムテンポを入力（オプション）
5. 「プレビュー」でノート情報を確認
6. 「再生」で音声再生をテスト
7. 「変換」でC言語コード生成
8. テキストエリアのコードをコピー

#### ファイル情報の理解

- **Tempo**: MIDIのテンポ(BPM)（指定テンポがあれば併記）
- **Tracks**: トラック数
- **Parts**: 現在のトラックのパート数
- **Notes**: 各パートのノート数

### Python版(パワーユーザー向け)

#### インストール

```bash
pip install pretty-midi
```

#### 基本的な使用法

```bash
# 対話的に使用（シェルスクリプト）
./convert_py.sh

# 直接コマンド実行
python3 midi_to_spike.py song.mid 0

# テンポを指定
python3 midi_to_spike.py song.mid 0 --tempo 140

# 全トラック変換
python3 midi_to_spike.py song.mid --all --fast --overwrite --tempo 160
```

#### オプション

| オプション | 説明 |
|-----------|------|
| `--all` | 全トラックを変換 |
| `--fast` | 確認ダイアログをスキップ |
| `--overwrite` / `--yes` | 既存ファイルを自動上書き |
| `--tempo BPM` | カスタムテンポを指定 |

#### シェルスクリプト convert_py.sh

```bash
chmod +x convert_py.sh
./convert_py.sh
```

実行時の流れ:
1. MIDIファイル一覧表示
2. ファイル番号選択
3. テンポ入力（空白で自動検出）
4. 特定トラック/全トラック選択
5. トラック番号入力（特定トラック選択時）
6. 変換実行

### C言語版(開発者向け)

#### コンパイル

```bash
gcc -o main main.c -lm
```

#### 基本的な使用法

```bash
# 対話的に使用（シェルスクリプト）
./convert_c.sh

# 直接コマンド実行
./main song.mid 0

# テンポを指定
./main song.mid 0 --tempo 140

# 全トラック変換
./main song.mid --all --fast --overwrite --tempo 160
```

#### オプション

| オプション | 説明 |
|-----------|------|
| `--all` | 全トラックを変換 |
| `--fast` | 確認ダイアログをスキップ |
| `--overwrite` / `--yes` | 既存ファイルを自動上書き |
| `--tempo BPM` | カスタムテンポを指定 |

#### シェルスクリプト convert_c.sh

```bash
chmod +x convert_c.sh
./convert_c.sh
```

実行時の流れ:
1. MIDIファイル一覧表示
2. ファイル番号選択
3. テンポ入力（空白で自動検出）
4. 特定トラック/全トラック選択
5. トラック番号入力（特定トラック選択時）
6. 変換実行

---

## テンポカスタマイズ

すべてのバージョンでテンポ（BPM）をカスタマイズできます。

### Web版
- 「テンポ(BPM)」フィールドに値を入力
- 空白で自動検出

### コマンドライン版
```bash
# C版
./main song.mid 0 --tempo 140

# Python版
python3 midi_to_spike.py song.mid 0 --tempo 140
```

### シェルスクリプト版
```bash
# 実行時にテンポ入力プロンプトが表示
./convert_c.sh
# Enter custom tempo in BPM (leave empty for auto-detect): 140
```

詳細は `TEMPO_CUSTOMIZATION.md` を参照してください。

---

## 出力ファイルの管理

### 生成ファイルの場所

```bash
song_midi/                    # MIDIファイル名に基づいて生成
├── track0/
│   ├── part0/
│   │   ├── output.txt      # SPIKE RT C code
│   │   └── play.html       # ブラウザプレビュー
│   └── part1/
│       ├── output.txt
│       └── play.html
├── track1/
│   └── part0/
│       ├── output.txt
│       └── play.html
└── play.html               # マルチトラック統合プレイヤー
```

### ファイルの説明

| ファイル | 説明 |
|----------|------|
| `output.txt` | SPIKE RT用C言語コード |
| `play.html` | 個別パートのブラウザプレイヤー |
| `play.html` (ルート) | 全パート再生可能な統合プレイヤー |

### ファイルの削除

```bash
# 生成ファイルをクリア
rm -rf song_midi/

# 再度変換可能
python3 midi_to_spike.py song.mid --all
```

---

## トラブルシューティング

### Web版

| 問題 | 解決方法 |
|------|---------|
| MIDIが読み込めない | ファイル形式がMIDI (.mid, .midi)か確認 |
| 音が出ない | ブラウザのスピーカーが有効か、音量確認 |
| コードが生成されない | ブラウザコンソールでエラー確認 (F12) |
| テンポが反映されない | 数値が正しいか、1-500の範囲か確認 |

### Python版

| 問題 | 解決方法 |
|------|---------|
| `pretty_midi not found` | `pip install pretty-midi` を実行 |
| ファイルが見つからない | MIDIファイルのパス確認 |
| テンポが読み取れない | MIDIファイルが有効か確認 |
| --tempo が認識されない | 形式確認: `--tempo 数字` |

### C言語版

| 問題 | 解決方法 |
|------|---------|
| コンパイルエラー | `gcc --version` でgccが インストール済みか確認 |
| メモリエラー | MIDIファイルを分割 |
| ファイルが見つからない | カレントディレクトリ確認 |
| --tempo が認識されない | 形式確認: `--tempo 数字` |

---

## 高度な設定

### 複数ファイルの一括処理

```bash
# 全MIDIファイルを変換（Python版）
for file in *.mid; do
    python3 midi_to_spike.py "$file" --all --fast --overwrite
done

# テンポ指定で一括処理
for file in *.mid; do
    python3 midi_to_spike.py "$file" --all --fast --overwrite --tempo 140
done
```

### テンポの確認

生成されたコードにテンポが含まれます：

```c
const int tempo = 140;  // BPM
const int beat_ms = 60000 / tempo;
```

---

## 次のステップ

1. サンプルで動作確認
2. 自分のMIDIファイルで試す
3. 生成されたコードをSPIKE RTプロジェクトに組み込む

詳細は各ドキュメントを参照：
- `README.md` - 機能説明
- `WEB_CONVERTER_README.md` - Web版詳細ガイド
- `TEMPO_CUSTOMIZATION.md` - テンポカスタマイズ
