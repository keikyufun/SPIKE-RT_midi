# SPIKE-RT-MIDI Converter

This project is a converter that analyzes MIDI files and converts them into `hub_speaker_play_tone` and `hub_speaker_set_volume` functions for SPIKE RT.

## Versions
- **C Version**: Command-line program.
- **HTML Version**: Web browser interface.
- **Python Version**: Command-line version using the `pretty-midi` library (Recommended).

## Usage (Python Version)
Based on the `pretty-midi` library.

1. Ensure Python 3 is installed.
2. Run `./convert_py.sh`.
3. Select a track from the displayed list.
4. Enter the track number to convert.
5. C code will be generated in `output.txt`.

## Usage (C Version)
1. Ensure a C compiler (gcc) is installed.
2. Run `./convert_c.sh`.
3. Select a track from the list.
4. Enter the track number.
5. C code will be generated in `output.txt`.

## Usage (HTML Version)
1. Open `index.html` in a web browser.
2. Upload a MIDI file.
3. Click "Load MIDI File".
4. Select a track.
5. Use "Preview Track" to check note info or "Play Track" to listen via Web Audio API.
6. Click "Convert" to display the code in the text area.

## Output Example
Example of generated code:

```c
// Generated code for SPIKE RT
#include <spike/hub/speaker.h>

void play_midi() {
    hub_speaker_set_volume(100); // Default volume
    hub_speaker_set_volume(64);
    hub_speaker_play_tone(440, 500);
    // ...
}
```

## Notes
- Tempo is assumed to be 120 BPM (C/HTML versions).
- Python version considers the actual MIDI tempo.
- Note-off handling is simplified.
- To use in a real SPIKE RT environment, integrate the generated code into your project.

## Compilation (C Version)
```bash
gcc main.c -o midi_converter -lm
```

## Dependencies (Python Version)
- Python 3
- `pretty-midi` / `mido`, `numpy`

## Shell Script Usage

### 1. Grant Permissions
Run this once:
```bash
chmod +x convert_c.sh convert_py.sh
```

### 2. Execution
Start the script:
```bash
./convert_py.sh
```

### 3. Operation
The script lists `.mid` files in the current directory.
- **Single File**: Enter the file number, then the track number (default 0).
- **All Files**: Type `all` to convert everything using `--fast --overwrite` mode.

### 4. File Structure
- `convert_py.sh`: The shell script.
- `midi_to_spike.py`: The core Python converter.

## Options & Examples
- C Version: `./midi_converter <midi> [track] [--all] [--fast] [--overwrite]`
- Python Version: `python3 midi_to_spike.py <midi> [track] [--all] [--fast] [--overwrite]`

#### Example Commands
```bash
# Convert all tracks of song.mid quickly
python3 midi_to_spike.py song.mid --all --fast --overwrite
```

## Generated Files
- `<midi_name>/track<no>/output.txt` - SPIKE RT Code
- `<midi_name>/track<no>/play.html` - Browser preview HTML
