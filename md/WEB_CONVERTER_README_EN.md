# Web Converter Tool Guide

Detailed guide for the browser-based SPIKE-RT MIDI converter tool.

## Overview

The web converter tool (convert.html) is a browser application that converts MIDI audio into C language code for SPIKE-RT projects.

### Features

- **MIDI File Upload**: Select MIDI from file system
- **Track Selection**: Process individual tracks from multi-track MIDI
- **Part Splitting**: Auto-split chords into multiple parts
- **Tempo Extraction - **Tempo Extraction**: Customization**: Auto-detect MIDI embedded tempo
- **Audio Preview**: Playback verification via Web Audio API
- **C Code Generation**: Automatic SPIKE-RT compatible code generation
- **Multiple Output Formats**: Single part/all parts conversion
- **Dark Green Theme**: User-friendly UI design

---

## Usage

### Basic Workflow

#### 1. Load File

```
Click [Load MIDI File] button
  -> File dialog appears
  -> Select MIDI file
  -> File information displays
```

Displayed information:
- **Tempo (BPM)**: Tempo set in MIDI
- **Tracks**: Number of tracks
- **Current Track**: Currently selected track
- **Parts**: Number of parts in current track
- **Notes**: Number of notes in each part

#### 2. Select Track

```
Select from Track dropdown
  -> Preview info auto-updates
  -> Part count and note info changes
```

#### 3. Preview Data

```
Click [Preview] button
  -> Note information displays
  -> View details for each part
```

Display content:
- Note list per part
- Each note start time (ms)
- Note length (ms)
- Frequency (Hz)

#### 4. Audio Test

```
Click [Play] button
  -> Playback via Web Audio API
  -> Audio output from browser speaker
```

#### 5. Generate Code

```
Click [Convert] button
  -> Auto-generate C language code
  -> Display in text box
```

#### 6. Review and Copy Code

```
Review code in text box
  -> Select all with Ctrl+A
  -> Copy with Ctrl+C
  -> Paste in text editor
```

---

## Feature Details

### Track Processing

#### Single Track Conversion

```
1. Select track from dropdown
2. Click [Convert]
3. Code generated for 1 track
```

#### All Tracks Conversion

Web version supports single track processing only.
For batch processing, use Python or C version.

```bash
# Convert all tracks with Python version
python3 midi_to_spike.py song.mid --all --fast

# Convert all tracks with C version
./main song.mid --all --fast
```

### Chord Splitting (Auto Part Generation)

When chords (multiple simultaneous notes) are detected in a track, they are automatically split into multiple parts.

**Example:**
```
Original track: C4 E4 G4 (simultaneous)
     |
     v
 After split:
 Part 0: C4
 Part 1: E4
 Part 2: G4
```

Each part is generated as separate code lines.

### Tempo Processing

#### Auto Tempo Extraction

Extracted from "Set Tempo" meta-event (FF 51 03) in MIDI.

```
Formula: BPM = 60,000,000 / microseconds per beat
```

#### Tempo Not Found

Default: 120 BPM

#### Tempo in Generated Code

```c
const int tempo = 170;  // BPM
const int beat_ms = 60000 / tempo;  // 1 beat = 353ms
```

---

## Output Code Format

### Single Part Case

```c
const int tempo = 170;
const int beat_ms = 353;
const int note_data[][2] = {
    {440, 375},
    {494, 375},
    {523, 750},
    {0, 0}
};
```

### Multiple Parts Case

Each part is generated separately:

```c
/* Track 0 Part 0 */
const int tempo = 170;
const int beat_ms = 353;
const int note_data[][2] = {
    {440, 375},
    {494, 375},
    {0, 0}
};

/* Track 0 Part 1 */
const int tempo = 170;
const int beat_ms = 353;
const int note_data[][2] = {
    {523, 750},
    {587, 375},
    {0, 0}
};
```

### Code Element Descriptions

| Element | Description |
|---------|-------------|
| `tempo` | Tempo in MIDI (BPM) |
| `beat_ms` | Length of 1 beat (milliseconds) |
| `note_data` | Note information |
| `note_data[i][0]` | Frequency (Hz) |
| `note_data[i][1]` | Note length (ms) |
| `{0, 0}` | List terminator |

---

## MIDI Note Information

### MIDI Note Number to Frequency Conversion

```
Frequency = 440 * 2^((note_number - 69) / 12)
```

### Common Frequencies

| Note | Frequency (Hz) |
|------|----------------|
| C4 | 262 |
| D4 | 294 |
| E4 | 330 |
| F4 | 349 |
| G4 | 392 |
| A4 | 440 |
| B4 | 494 |
| C5 | 523 |

---

## Audio Preview Usage

### Web Audio API Support

```
Supported browsers:
- Chrome/Chromium
- Firefox
- Safari
- Edge
```

### Limitations

- Volume is fixed (adjust via system volume)
- Waveform is sine wave
- Real-time effects not supported

### Troubleshooting

| Problem | Solution |
|---------|----------|
| No sound | Verify browser speaker enabled |
| Distorted audio | Check MIDI note information |
| Playback stops | Verify MIDI file format |

---

## Browser Compatibility

### Recommended

- Chrome/Chromium 90+
- Firefox 88+
- Safari 14+
- Edge 90+

### Required Features

- FileReader API
- Web Audio API
- ES6 JavaScript

---

## Troubleshooting

### MIDI File Related

| Problem | Cause | Solution |
|---------|-------|----------|
| Cannot open file | File not in MIDI format | Check .mid or .midi extension |
| Info not displayed | Invalid MIDI header | Try different MIDI file |
| Tempo is 120 | Tempo not set | Check MIDI file |

### Playback Related

| Problem | Cause | Solution |
|---------|-------|----------|
| Silent | Browser volume | Check browser/system volume |
| Distorted sound | Invalid note info | Verify MIDI file |
| Stops mid-playback | Insufficient memory | Restart browser |

### Code Generation Related

| Problem | Cause | Solution |
|---------|-------|----------|
| Empty code | Track has no notes | Select different track |
| Tempo not reflected | MIDI tempo not detected | Default is 120 BPM |
| Too many notes | Memory limit | Split file |

---

## Usage Examples

### Example 1: Simple Melody

```
1. Upload sample.mid
2. Select Track 0
3. Click [Play] to verify
4. Click [Convert] to generate code
5. Copy code
```

### Example 2: File with Chords

```
1. Upload harmony.mid
2. Select Track 0
3. Multiple Parts displayed (chord split done)
4. Click [Preview] to view each part
5. Click [Convert] to generate code (per part)
```

### Example 3: Fast Melody

```
1. Upload fast_song.mid
2. Click [Preview] to view note info
3. If many notes: Click [Convert] to generate code
4. Or use Python: python3 midi_to_spike.py fast_song.mid
```

---

## Next Steps

1. **Integrate Code**: Integrate generated code into SPIKE-RT project
2. **Test**: Verify operation on SPIKE-RT board
3. **Adjust**: Fine-tune tempo or note length as needed
4. **Multi-Track**: Use Python version for multi-track processing

See `README_EN.md` for more details.
