#!/bin/bash
set -e

MIDI_FILES=( *.mid )
if [ ${#MIDI_FILES[@]} -eq 0 ]; then
    echo "No MIDI files found."
    exit 1
fi

echo "MIDI files list:" 
for i in "${!MIDI_FILES[@]}"; do
    echo "[$i] ${MIDI_FILES[$i]}"
done

read -p "Select number (all = all): " sel

# Prompt for custom tempo
read -p "Enter custom tempo in BPM (leave empty for auto-detect): " custom_tempo

# Build tempo option if provided
TEMPO_OPT=""
if [ -n "$custom_tempo" ] && [[ "$custom_tempo" =~ ^[0-9]+$ ]]; then
    TEMPO_OPT="--tempo $custom_tempo"
fi

process_file() {
    local file="$1"
    local track_index="$2"

    echo "--- Processing $file ---"
    python3 midi_to_spike.py "$file" "$track_index" $TEMPO_OPT
    echo "$file: Done"
}

if [ "$sel" == "all" ]; then
    for file in "${MIDI_FILES[@]}"; do
        echo "--- Converting $file (all tracks, fast mode with auto-overwrite) ---"
        python3 midi_to_spike.py "$file" --all --fast --overwrite $TEMPO_OPT
    done
else
    if ! [[ "$sel" =~ ^[0-9]+$ ]] || [ "$sel" -ge "${#MIDI_FILES[@]}" ]; then
        echo "Invalid number"
        exit 1
    fi

    file="${MIDI_FILES[$sel]}"
    
    echo "--- Track selection for $file ---"
    echo "[0] Specific track"
    echo "[1] All tracks"
    read -p "Select (0 or 1): " track_opt
    
    if [ "$track_opt" == "1" ]; then
        echo "--- Converting $file (all tracks, fast mode) ---"
        python3 midi_to_spike.py "$file" --all --fast --overwrite $TEMPO_OPT
    else
        read -p "Enter track number for $file (default 0): " track
        track=${track:-0}
        echo "--- Converting $file track $track (fast mode) ---"
        python3 midi_to_spike.py "$file" "$track" --fast $TEMPO_OPT
    fi
fi
