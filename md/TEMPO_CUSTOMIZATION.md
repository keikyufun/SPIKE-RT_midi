# テンポカスタマイズガイド

SPIKE-RT-MIDI Converterはすべてのバージョン（HTML、C、Python）でテンポ（BPM）をカスタマイズできます。

## Web版（convert.html）

### 使用方法

1. MIDIファイルを読み込む
2. 「テンポ(BPM)」フィールドにカスタムテンポを入力
3. 空白にするとMIDIから自動検出
4. 変換ボタンでコード生成

### 例

```
MIDI内テンポ: 120 BPM
入力: 140 BPM
→ 出力コードは140 BPMで生成
```

## C言語版

### コマンドラインオプション

```bash
./main <midi_file> [track_number] [--tempo BPM] [--all] [--fast] [--overwrite]
```

### 例

```bash
# 単一トラックを140 BPMで変換
./main song.mid 0 --tempo 140

# 全トラックを140 BPMで変換（確認なし）
./main song.mid --all --tempo 140 --fast

# デフォルトテンポで変換（オプションなし）
./main song.mid 0
```

### テンポ表示

```
Number of tracks: 1
Tempo: 120 BPM (Custom: 140 BPM)
```

## Python版

### コマンドラインオプション

```bash
python3 midi_to_spike.py <midi_file> [track_number] [--tempo BPM] [--all] [--fast] [--overwrite]
```

### 例

```bash
# 単一トラックを140 BPMで変換
python3 midi_to_spike.py song.mid 0 --tempo 140

# 全トラックを160 BPMで変換（確認なし）
python3 midi_to_spike.py song.mid --all --tempo 160 --fast

# デフォルトテンポで変換
python3 midi_to_spike.py song.mid 0
```

### テンポ表示

```
Number of tracks: 1
Tempo: 120 BPM (Custom: 140 BPM)
```

## テンポの影響

### 有効な値

- 最小: 1 BPM
- 最大: 500 BPM（推奨上限）

### 注意事項

- ノートの長さは変わりません（ミリ秒単位のまま）
- テンポ変更はコードに埋め込まれます
- SPIKE RTで速度調整が必要な場合に使用

## 生成コードの例

### テンポが組み込まれたコード

```c
const int tempo = 140;  // BPM（カスタム指定）
const int beat_ms = 60000 / tempo;  // 1ビート = 428ms
const int note_data[][2] = {
    {440, 375},
    {494, 375},
    {523, 750},
    {0, 0}
};
```

## トラブルシューティング

### HTML版

| 問題 | 解決方法 |
|------|---------|
| テンポが反映されない | ブラウザのコンソール確認 (F12) |
| 無効な値 | 1-500の範囲で入力 |

### C/Python版

| 問題 | 解決方法 |
|------|---------|
| --tempo が認識されない | バージョン確認（最新版か） |
| 数値エラー | 正の整数を入力 |

## 使用例

### 例1: 遅いMIDIを高速化

```bash
# 元のテンポ: 60 BPM（遅い）
python3 midi_to_spike.py slow_song.mid --tempo 120
# 2倍速で再生
```

### 例2: 高速なMIDIを低速化

```bash
# 元のテンポ: 240 BPM（速い）
./main fast_song.mid --all --tempo 160 --fast
# 速度を低下
```

### 例3: Web版でプレビュー

```
1. slow_song.mid を Web版で読み込む
2. テンポ: 60 BPM (自動検出)
3. 「テンポ(BPM)」に 120 入力
4. 「プレビュー」で確認
5. 「変換」でコード生成
```

## 仕組み

### テンポから計算される値

```
BPM = 120
beat_ms = 60000 / 120 = 500ms
```

### コード内での使用

```c
// テンポ値の計算
const int beat_ms = 60000 / tempo;

// ビート単位のタイミング計算
int delay_ms = beat_count * beat_ms;
```

## 次のステップ

1. Web版またはCLI版で異なるテンポを試す
2. SPIKE RTで動作確認
3. 最適なテンポを見つける
