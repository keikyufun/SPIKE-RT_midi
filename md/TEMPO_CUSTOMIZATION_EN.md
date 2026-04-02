# Tempo Customization Guide

The SPIKE-RT-MIDI Converter allows custom tempo (BPM) specification in all versions (HTML, C, Python).

## Web Version (convert.html)

### Usage

1. Load MIDI file
2. Enter custom tempo in "Tempo (BPM)" field
3. Leave blank to auto-detect from MIDI
4. Click convert button to generate code

### Example

```
MIDI tempo: 120 BPM
Input: 140 BPM
Output: Code generated with 140 BPM
```

## C Language Version

### Command-line Options

```bash
./main <midi_file> [track_number] [--tempo BPM] [--all] [--fast] [--overwrite]
```

### Examples

```bash
# Convert single track with 140 BPM
./main song.mid 0 --tempo 140

# Convert all tracks with 140 BPM (no confirmation)
./main song.mid --all --tempo 140 --fast

# Convert with default tempo (no option)
./main song.mid 0
```

### Tempo Display

```
Number of tracks: 1
Tempo: 120 BPM (Custom: 140 BPM)
```

## Python Version

### Command-line Options

```bash
python3 midi_to_spike.py <midi_file> [track_number] [--tempo BPM] [--all] [--fast] [--overwrite]
```

### Examples

```bash
# Convert single track with 140 BPM
python3 midi_to_spike.py song.mid 0 --tempo 140

# Convert all tracks with 160 BPM (no confirmation)
python3 midi_to_spike.py song.mid --all --tempo 160 --fast

# Convert with default tempo
python3 midi_to_spike.py song.mid 0
```

### Tempo Display

```
Number of tracks: 1
Tempo: 120 BPM (Custom: 140 BPM)
```

## Tempo Impact

### Valid Values

- Minimum: 1 BPM
- Maximum: 500 BPM (recommended limit)

### Important Notes

- Note length remains unchanged (in milliseconds)
- Tempo change is embedded in generated code
- Use when speed adjustment needed for SPIKE RT

## Generated Code Example

### Code with Embedded Tempo

```c
const int tempo = 140;  // BPM (custom specified)
const int beat_ms = 60000 / tempo;  // 1 beat = 428ms
const int note_data[][2] = {
    {440, 375},
    {494, 375},
    {523, 750},
    {0, 0}
};
```

## Troubleshooting

### Web Version

| Problem | Solution |
|---------|----------|
| Tempo not reflected | Check browser console (F12) |
| Invalid value | Enter value in 1-500 range |

### C/Python Version

| Problem | Solution |
|---------|----------|
| --tempo not recognized | Verify version (latest?) |
| Number error | Enter positive integer |

## Use Cases

### Example 1: Speed Up Slow MIDI

```bash
# Original tempo: 60 BPM (slow)
python3 midi_to_spike.py slow_song.mid --tempo 120
# Play at double speed
```

### Example 2: Slow Down Fast MIDI

```bash
# Original tempo: 240 BPM (fast)
./main fast_song.mid --all --tempo 160 --fast
# Reduce speed
```

### Example 3: Web Version Preview

```
1. Load slow_song.mid in web version
2. Display: Tempo: 60 BPM (auto-detected)
3. Enter 120 in "Tempo (BPM)" field
4. Click "Preview" to verify
5. Click "Convert" to generate code
```

## How It Works

### Calculation from Tempo

```
BPM = 120
beat_ms = 60000 / 120 = 500ms
```

### Usage in Code

```c
// Calculate tempo value
const int beat_ms = 60000 / tempo;

// Calculate beat-based timing
int delay_ms = beat_count * beat_ms;
```

## Next Steps

1. Try different tempos with web version or CLI
2. Test on SPIKE RT board
3. Find optimal tempo
