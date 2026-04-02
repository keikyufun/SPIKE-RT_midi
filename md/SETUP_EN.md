# Setup Guide

This guide explains how to set up the SPIKE-RT-MIDI Converter.

## Prerequisites

### For Web Version
- Modern Web browser
- Python 3.x or Node.js

### For Python Version
- Python 3.6 or later
- pretty-midi library
- numpy

### For C Language Version
- C compiler (gcc, clang, etc.)
- Linux/Mac/Windows (WSL) environment

---

## Quick Start

### Step 1: Verify Files

```bash
# Navigate to project directory
cd spike-rt-midi

# Check file list
ls -la
```

Confirm these files exist:
- `convert.html` - Main web interface
- `midi_to_spike.py` - Python conversion script
- `convert_py.sh` - Python shell script
- `main.c` - C language source
- `convert_c.sh` - C language shell script
- `DREAM.mid` - Sample MIDI file

### Step 2: Open Web Version

#### Method A: Open directly in browser

```bash
# On Mac
open convert.html

# On Linux
xdg-open convert.html

# Or double-click convert.html in file manager
```

#### Method B: Via local server (Recommended)

**Python 3:**
```bash
python3 -m http.server 8000
```

**Node.js:**
```bash
npx http-server
```

Open in browser:
```
http://localhost:8000/convert.html
```

### Step 3: Set Up Python Version (Optional)

```bash
# Install pretty-midi
pip install pretty-midi

# Test run
python3 midi_to_spike.py DREAM.mid --fast
```

### Step 4: Set Up C Version (Optional)

```bash
# Compile
gcc -o main main.c -lm

# Test run
./main DREAM.mid --fast
```

---

## Detailed Setup

### File Structure

```
spike-rt-midi/
├── convert.html              Web interface
├── midi_to_spike.py         Python conversion script
├── convert_py.sh            Python shell script
├── main.c                   C language source
├── convert_c.sh             C language shell script
├── main                     Compiled C executable
├── README.md                Main documentation (Japanese)
├── README_EN.md             English documentation
├── SETUP.md                 This file (Japanese)
├── SETUP_EN.md              English setup guide
├── WEB_CONVERTER_README.md  Web version guide (Japanese)
├── WEB_CONVERTER_README_EN.md Web version guide (English)
├── TEMPO_CUSTOMIZATION.md   Tempo customization (Japanese)
├── TEMPO_CUSTOMIZATION_EN.md Tempo customization (English)
├── DREAM.mid                Sample MIDI file
├── example.mid              Another sample MIDI
├── pretty-midi/             Python library
└── examples/                Documentation samples
```

---

## Usage for Each Version

### Web Version (Recommended for beginners)

#### Basic workflow

1. Open `convert.html` in browser
2. Click "Load MIDI File" button to select MIDI file
3. Select track from dropdown
4. Enter custom tempo in "Tempo (BPM)" field (Optional)
5. Click "Preview" to view note information
6. Click "Play" to test audio playback
7. Click "Convert" to generate C code
8. Copy code from text area

#### Understanding file information

- **Tempo**: MIDI tempo in BPM (with custom if specified)
- **Tracks**: Number of tracks
- **Parts**: Number of parts in current track
- **Notes**: Number of notes in each part

### Python Version (For power users)

#### Installation

```bash
pip install pretty-midi
```

#### Basic usage

```bash
# Interactive mode (shell script)
./convert_py.sh

# Direct command execution
python3 midi_to_spike.py song.mid 0

# Specify custom tempo
python3 midi_to_spike.py song.mid 0 --tempo 140

# Convert all tracks
python3 midi_to_spike.py song.mid --all --fast --overwrite --tempo 160
```

#### Options

| Option | Description |
|--------|-------------|
| `--all` | Convert all tracks |
| `--fast` | Skip confirmation dialogs |
| `--overwrite` / `--yes` | Auto-overwrite existing files |
| `--tempo BPM` | Specify custom tempo |

#### Shell Script convert_py.sh

```bash
chmod +x convert_py.sh
./convert_py.sh
```

Execution flow:
1. Display MIDI file list
2. Select file number
3. Enter custom tempo (or leave blank for auto-detect)
4. Select specific track or all tracks
5. Enter track number (if specific track selected)
6. Execute conversion

### C Language Version (For developers)

#### Compilation

```bash
gcc -o main main.c -lm
```

#### Basic usage

```bash
# Interactive mode (shell script)
./convert_c.sh

# Direct command execution
./main song.mid 0

# Specify custom tempo
./main song.mid 0 --tempo 140

# Convert all tracks
./main song.mid --all --fast --overwrite --tempo 160
```

#### Options

| Option | Description |
|--------|-------------|
| `--all` | Convert all tracks |
| `--fast` | Skip confirmation dialogs |
| `--overwrite` / `--yes` | Auto-overwrite existing files |
| `--tempo BPM` | Specify custom tempo |

#### Shell Script convert_c.sh

```bash
chmod +x convert_c.sh
./convert_c.sh
```

Execution flow:
1. Display MIDI file list
2. Select file number
3. Enter custom tempo (or leave blank for auto-detect)
4. Select specific track or all tracks
5. Enter track number (if specific track selected)
6. Execute conversion

---

## Tempo Customization

Customize tempo (BPM) in all versions.

### Web Version
- Enter value in "Tempo (BPM)" field
- Leave blank for auto-detect

### Command-line Version
```bash
# C version
./main song.mid 0 --tempo 140

# Python version
python3 midi_to_spike.py song.mid 0 --tempo 140
```

### Shell Script Version
```bash
# Tempo input prompt displays during execution
./convert_c.sh
# Enter custom tempo in BPM (leave empty for auto-detect): 140
```

See `TEMPO_CUSTOMIZATION_EN.md` for details.

---

## Output File Management

### Generated files location

```bash
song_mid/                    # Generated based on MIDI filename
├── track0/
│   ├── part0/
│   │   ├── output.txt      # SPIKE RT C code
│   │   └── play.html       # Browser preview
│   └── part1/
│       ├── output.txt
│       └── play.html
├── track1/
│   └── part0/
│       ├── output.txt
│       └── play.html
└── play.html               # Multi-track unified player
```

### File descriptions

| File | Description |
|------|-------------|
| `output.txt` | C code for SPIKE RT |
| `play.html` | Individual part browser player |
| `play.html` (root) | Unified player for all parts |

### Deleting generated files

```bash
# Clear generated files
rm -rf song_midi/

# Can convert again
python3 midi_to_spike.py song.mid --all
```

---

## Troubleshooting

### Web Version

| Problem | Solution |
|---------|----------|
| MIDI won't load | Verify file format is MIDI (.mid, .midi) |
| No sound | Check browser speaker is enabled and volume |
| Code not generating | Check browser console for errors (F12) |
| Tempo not reflected | Verify number is correct, 1-500 range |

### Python Version

| Problem | Solution |
|---------|----------|
| `pretty_midi not found` | Run `pip install pretty-midi` |
| File not found | Verify MIDI file path |
| Tempo not detected | Verify MIDI file is valid |
| --tempo not recognized | Verify format: `--tempo number` |

### C Language Version

| Problem | Solution |
|---------|----------|
| Compilation error | Verify gcc installed: `gcc --version` |
| Memory error | Split MIDI file |
| File not found | Check current directory |
| --tempo not recognized | Verify format: `--tempo number` |

---

## Advanced Configuration

### Batch processing multiple files

```bash
# Convert all MIDI files (Python version)
for file in *.mid; do
    python3 midi_to_spike.py "$file" --all --fast --overwrite
done

# Batch processing with custom tempo
for file in *.mid; do
    python3 midi_to_spike.py "$file" --all --fast --overwrite --tempo 140
done
```

### Checking tempo

Generated code includes tempo information:

```c
const int tempo = 140;  // BPM
const int beat_ms = 60000 / tempo;
```

---

## Next Steps

1. Verify functionality with sample
2. Try with your own MIDI file
3. Integrate generated code into SPIKE RT project

For more details, see:
- `README_EN.md` - Feature documentation
- `WEB_CONVERTER_README_EN.md` - Web version detailed guide
- `TEMPO_CUSTOMIZATION_EN.md` - Tempo customization
