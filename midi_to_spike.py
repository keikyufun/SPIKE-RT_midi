#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import os
# sys.path.insert(0, os.path.join(os.path.dirname(__file__), 'pretty-midi'))

import pretty_midi
import math

def midi_note_to_frequency(note):
    """MIDIノート番号から周波数を計算"""
    return 440.0 * math.pow(2.0, (note - 69) / 12.0)

def get_tempo_from_midi(midi_file):
    """MIDIファイルからテンポを取得（BPM）"""
    pm = pretty_midi.PrettyMIDI(midi_file)
    tempo_changes = pm.get_tempo_changes()
    if tempo_changes[0].size > 0:
        return int(tempo_changes[1][0])
    return 120  # デフォルト120BPM

def write_track_html(html_file, notes, tempo=120):
    """単一トラックのHTMLを生成"""
    with open(html_file, "w", encoding="utf-8") as f:
        f.write("""<!DOCTYPE html>
<html lang="ja">
<head>
    <meta charset="utf-8">
    <title>SPIKE-RT-MIDI Track Player</title>
    <style>
        body { font-family: Arial, sans-serif; max-width: 900px; margin: 50px auto; padding: 20px; background-color: #f5f5f5; }
        h1 { color: #333; }
        .container { background-color: white; padding: 20px; border-radius: 8px; box-shadow: 0 2px 8px rgba(0,0,0,0.1); }
        button { padding: 10px 20px; margin: 10px 5px 10px 0; background-color: #667eea; color: white; border: none; border-radius: 5px; cursor: pointer; }
        button:hover { background-color: #764ba2; }
        pre { background-color: #f0f0f0; padding: 15px; border-radius: 5px; overflow-x: auto; font-size: 12px; }
        .note-info { margin: 10px 0; }
        .duration { color: #e74c3c; font-weight: bold; }
    </style>
</head>
<body>
    <div class="container">
        <h1> SPIKE-RT-MIDI Player</h1>
        <div class="note-info">
            <strong>合計ノート数:</strong> """ + str(len(notes)) + """<br>
            <strong>テンポ:</strong> """ + str(tempo) + """ BPM<br>
            <strong>総再生時間:</strong> """ + str(sum(n['dur'] for n in notes)) + """ms
        </div>
        <button onclick="playTrack()">▶️ 再生</button>
        <button onclick="stopTrack()">️ 停止</button>
        <pre id="notes"></pre>
    </div>
    <script>
        const notes = [""")
        for n in notes:
            f.write(f"            {{freq:{n['freq']}, dur:{n['dur']}, vol:{n['vol']}, start:{n['start']}}},\n")
        f.write("""        ];
        const tempo = """ + str(tempo) + """;
        
        const ctx = new (window.AudioContext || window.webkitAudioContext)();
        let playingOscillators = [];
        
        function displayNotes() {
            const html = notes.map((n, i) => 
                `<span style="color: #333;">ノート ${i + 1}: 周波数=<strong>${n.freq}Hz</strong> | 音量=<span style="color: #3498db;"><strong>${n.vol}%</strong></span> | 長さ=<span class="duration">${n.dur}ms</span> | 開始=${n.start}ms</span>`
            ).join('<br>');
            document.getElementById('notes').innerHTML = html;
        }
        
        function playTrack() {
            ctx.resume().then(() => {
                stopTrack();
                let base = ctx.currentTime;
                notes.forEach(n => {
                    if (n.dur <= 0) return;
                    const o = ctx.createOscillator();
                    const g = ctx.createGain();
                    o.connect(g);
                    g.connect(ctx.destination);
                    o.frequency.value = n.freq;
                    o.type = 'sine';
                    const start = base + n.start / 1000;
                    const end = start + n.dur / 1000;
                    g.gain.setValueAtTime(n.vol / 100 * 0.15, start);
                    g.gain.setValueAtTime(0, end);
                    o.start(start);
                    o.stop(end);
                    playingOscillators.push(o);
                });
            });
        }
        
        function stopTrack() {
            playingOscillators.forEach(o => {
                try { o.stop(); } catch (e) { }
            });
            playingOscillators = [];
        }
        
        displayNotes();
    </script>
</body>
</html>""")

def write_all_tracks_html(html_file, all_notes, tempo=120):
    with open(html_file, "w", encoding="utf-8") as f:
        f.write("""<!DOCTYPE html>
<html lang="ja">
<head>
    <meta charset="utf-8">
    <title>SPIKE-RT-MIDI All Tracks Player</title>
    <style>
        body { font-family: Arial, sans-serif; max-width: 900px; margin: 50px auto; padding: 20px; background-color: #f5f5f5; }
        h1 { color: #333; }
        .container { background-color: white; padding: 20px; border-radius: 8px; box-shadow: 0 2px 8px rgba(0,0,0,0.1); }
        select, button { padding: 10px 15px; margin: 10px 5px 10px 0; }
        button { background-color: #667eea; color: white; border: none; border-radius: 5px; cursor: pointer; }
        button:hover { background-color: #764ba2; }
        pre { background-color: #f0f0f0; padding: 15px; border-radius: 5px; overflow-x: auto; font-size: 12px; max-height: 400px; overflow-y: auto; }
        .duration { color: #e74c3c; font-weight: bold; }
        .stats { background-color: #ecf0f1; padding: 10px; border-radius: 5px; margin: 10px 0; }
    </style>
</head>
<body>
    <div class="container">
        <h1> SPIKE-RT-MIDI All Tracks Player</h1>
        <label for="trackSelect"><strong>トラック選択:</strong></label>
        <select id="trackSelect" onchange="updatePreview()">""")
        for i, notes in enumerate(all_notes):
            total_dur = sum(n['dur'] for n in notes)
            f.write(f'            <option value="{i}">Track {i} ({len(notes)} notes, {total_dur}ms)</option>\n')
        f.write("""        </select>
        <button onclick="playTrack()">▶️ 再生</button>
        <button onclick="stopTrack()">️ 停止</button>
        <div class="stats">
            <div id="stats"></div>
        </div>
        <pre id="notes"></pre>
    </div>
    <script>
        const allTracks = [""")
        for notes in all_notes:
            f.write("            [\n")
            for n in notes:
                f.write(f"                {{freq:{n['freq']}, dur:{n['dur']}, vol:{n['vol']}, start:{n['start']}}},\n")
            f.write("            ],\n")
        f.write("""        ];
        const tempo = """ + str(tempo) + """;
        
        const ctx = new (window.AudioContext || window.webkitAudioContext)();
        let currentTrack = 0;
        let playingOscillators = [];
        
        function updatePreview() {
            currentTrack = parseInt(document.getElementById('trackSelect').value);
            const notes = allTracks[currentTrack] || [];
            const totalDur = notes.reduce((sum, n) => sum + n.dur, 0);
            document.getElementById('stats').innerHTML = `<strong>ノート数:</strong> ${notes.length} | <strong>総再生時間:</strong> ${totalDur}ms`;
            const html = notes.map((n, i) => 
                `<span style="color: #333;">ノート ${i + 1}: 周波数=<strong>${n.freq}Hz</strong> | 音量=<span style="color: #3498db;"><strong>${n.vol}%</strong></span> | 長さ=<span class="duration">${n.dur}ms</span></span>`
            ).join('<br>');
            document.getElementById('notes').innerHTML = html;
        }
        
        function playTrack() {
            ctx.resume().then(() => {
                stopTrack();
                const notes = allTracks[currentTrack] || [];
                let base = ctx.currentTime;
                notes.forEach(n => {
                    if (n.dur <= 0) return;
                    const o = ctx.createOscillator();
                    const g = ctx.createGain();
                    o.connect(g);
                    g.connect(ctx.destination);
                    o.frequency.value = n.freq;
                    o.type = 'sine';
                    const start = base + n.start / 1000;
                    const end = start + n.dur / 1000;
                    g.gain.setValueAtTime(n.vol / 100 * 0.15, start);
                    g.gain.setValueAtTime(0, end);
                    o.start(start);
                    o.stop(end);
                    playingOscillators.push(o);
                });
            });
        }
        
        function stopTrack() {
            playingOscillators.forEach(o => {
                try { o.stop(); } catch (e) { }
            });
            playingOscillators = [];
        }
        
        updatePreview();
    </script>
</body>
</html>""")


def separate_chords(notes):
    """和音を複数パートに分ける"""
    if not notes:
        return []
    
    parts = []
    sorted_notes = sorted(notes, key=lambda n: n.start)
    
    for note in sorted_notes:
        assigned = False
        for part in parts:
            can_add = True
            for existing_note in part:
                if not (note.end <= existing_note.start + 0.001 or note.start >= existing_note.end - 0.001):
                    can_add = False
                    break
            if can_add:
                part.append(note)
                assigned = True
                break
        
        if not assigned:
            parts.append([note])
    
    return parts

def convert_midi_to_spike(midi_file, selected_track=None, all_tracks=False, fast=False, overwrite=False, custom_tempo=None):
    """MIDIファイルをSPIKE RTコードに変換"""
    pm = pretty_midi.PrettyMIDI(midi_file)
    midi_tempo = get_tempo_from_midi(midi_file)
    tempo = custom_tempo if custom_tempo else midi_tempo

    print("Number of tracks:", len(pm.instruments))
    if not fast:
        print(f"Tempo: {midi_tempo} BPM", end="")
        if custom_tempo:
            print(f" (Custom: {tempo} BPM)")
        else:
            print()
    for i, inst in enumerate(pm.instruments):
        note_count = len(inst.notes)
        total_time = inst.notes[-1].end if note_count > 0 else 0
        if not fast:
            print(f"Track {i}: {note_count} notes, total time {total_time:.2f} seconds")

    if len(pm.instruments) == 0:
        print("No tracks found")
        return

    base = os.path.splitext(os.path.basename(midi_file))[0]
    os.makedirs(base, exist_ok=True)

    def process_track(idx):
        inst = pm.instruments[idx]
        parts = separate_chords(inst.notes)
        
        for part_idx, part_notes in enumerate(parts):
            notes = []
            code = """// SPIKE RT 用変換コード
#include <spike/hub/speaker.h>

void play_midi() {
    hub_speaker_set_volume(100); // デフォルト音量
"""
            sorted_part_notes = sorted(part_notes, key=lambda n: n.start)
            for note in sorted_part_notes:
                freq = int(midi_note_to_frequency(note.pitch))
                duration_ms = int((note.end - note.start) * 1000)
                vol = int(note.velocity * 100 / 127)
                code += f"    hub_speaker_set_volume({vol});\n"
                code += f"    hub_speaker_play_tone({freq}, {duration_ms});\n"
                notes.append({"freq": freq, "dur": duration_ms, "vol": vol, "start": int(note.start * 1000)})

            code += "}\n"

            track_dir = os.path.join(base, f"track{idx}", f"part{part_idx}")
            os.makedirs(track_dir, exist_ok=True)
            output_txt = os.path.join(track_dir, "output.txt")
            html_path = os.path.join(track_dir, "play.html")

            if not overwrite and not fast and (os.path.exists(output_txt) or os.path.exists(html_path)):
                ans = input(f"Existing files found: {output_txt} / {html_path}. Overwrite? (y/n): ")
                if ans.lower() != 'y':
                    print(f"Skipped: track{idx}/part{part_idx}")
                    return

            with open(output_txt, "w", encoding="utf-8") as f:
                f.write(code)

            write_track_html(html_path, notes, tempo)

            if not fast:
                print(f"Track{idx}/Part{part_idx} conversion completed: {output_txt}, {html_path}")

    if all_tracks:
        all_notes = []
        for i in range(len(pm.instruments)):
            inst = pm.instruments[i]
            parts = separate_chords(inst.notes)
            
            for part_idx, part_notes in enumerate(parts):
                notes = []
                code = """// SPIKE RT 用変換コード
#include <spike/hub/speaker.h>

void play_midi() {
    hub_speaker_set_volume(100); // デフォルト音量
"""
                sorted_part_notes = sorted(part_notes, key=lambda n: n.start)
                for note in sorted_part_notes:
                    freq = int(midi_note_to_frequency(note.pitch))
                    duration_ms = int((note.end - note.start) * 1000)
                    vol = int(note.velocity * 100 / 127)
                    code += f"    hub_speaker_set_volume({vol});\n"
                    code += f"    hub_speaker_play_tone({freq}, {duration_ms});\n"
                    notes.append({"freq": freq, "dur": duration_ms, "vol": vol, "start": int(note.start * 1000)})

                code += "}\n"

                track_dir = os.path.join(base, f"track{i}", f"part{part_idx}")
                os.makedirs(track_dir, exist_ok=True)
                output_txt = os.path.join(track_dir, "output.txt")
                html_path = os.path.join(track_dir, "play.html")

                if not overwrite and not fast and (os.path.exists(output_txt) or os.path.exists(html_path)):
                    ans = input(f"Existing files found: {output_txt} / {html_path}. Overwrite? (y/n): ")
                    if ans.lower() != 'y':
                        print(f"Skipped: track{i}/part{part_idx}")
                        continue

                with open(output_txt, "w", encoding="utf-8") as f:
                    f.write(code)

                write_track_html(html_path, notes, tempo)

                all_notes.append(notes)

                if not fast:
                    print(f"Track{i}/Part{part_idx} conversion completed: {output_txt}, {html_path}")

        # Write all tracks HTML
        all_html_path = os.path.join(base, "play.html")
        write_all_tracks_html(all_html_path, all_notes, tempo)
        if not fast:
            print(f"All tracks HTML: {all_html_path}")
        return

    if selected_track is None:
        while True:
            try:
                selected_track = int(input("Enter track number to convert: "))
                if 0 <= selected_track < len(pm.instruments):
                    break
                else:
                    print("Invalid number")
            except ValueError:
                print("Please enter a number")

    if selected_track < 0 or selected_track >= len(pm.instruments):
        print("Invalid track number")
        return

    process_track(selected_track)

def main():
    if len(sys.argv) < 2:
        print("Usage: python midi_to_spike.py <midi_file> [track_number] [--all] [--fast] [--overwrite|--yes] [--tempo BPM]")
        print("Example: python midi_to_spike.py song.mid 0")
        print("Example: python midi_to_spike.py song.mid --all --tempo 140")
        sys.exit(1)

    midi_file = sys.argv[1]
    if not os.path.exists(midi_file):
        print("File not found:", midi_file)
        sys.exit(1)

    selected_track = None
    all_tracks = False
    fast_mode = False
    overwrite = False
    custom_tempo = None

    i = 2
    while i < len(sys.argv):
        arg = sys.argv[i]
        if arg == "--all":
            all_tracks = True
        elif arg == "--fast":
            fast_mode = True
        elif arg == "--overwrite" or arg == "--yes":
            overwrite = True
        elif arg == "--tempo" and i + 1 < len(sys.argv):
            try:
                custom_tempo = int(sys.argv[i + 1])
                i += 1
            except ValueError:
                print("Error: --tempo requires a number")
                sys.exit(1)
        else:
            try:
                selected_track = int(arg)
            except ValueError:
                print("Unknown argument:", arg)
                sys.exit(1)
        i += 1

    convert_midi_to_spike(midi_file, selected_track, all_tracks, fast_mode, overwrite, custom_tempo)

if __name__ == "__main__":
    main()