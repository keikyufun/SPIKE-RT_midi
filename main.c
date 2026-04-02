#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <sys/stat.h>
#include <sys/types.h>
// #include <spike/hub/speaker.h> // 実際の環境でインクルード

// MIDIノート番号から周波数を計算
double midi_note_to_frequency(uint8_t note) {
    return 440.0 * pow(2.0, (note - 69) / 12.0);
}

// ノート構造体
typedef struct {
    uint8_t note;
    uint32_t start_time; // ティック
    uint32_t end_time;   // ティック
    uint8_t velocity;
} Note;

// パート構造体（複数ノートを含む）
typedef struct {
    Note *notes;
    int note_count;
    uint32_t total_time; // ミリ秒
} Part;

// トラック構造体（複数パートに分割可能）
typedef struct {
    Part *parts;
    int part_count;
} Track;

// MIDIファイル構造体
typedef struct {
    uint16_t format;
    uint16_t track_count;
    uint16_t division;
    uint16_t tempo; // BPM
    Track *tracks;
} MidiFile;

// 和音検出・分割関数
void separate_chords(Note *notes, int note_count, Part **parts_out, int *part_count_out) {
    if (note_count == 0) {
        *parts_out = NULL;
        *part_count_out = 0;
        return;
    }
    
    Part *parts = malloc(sizeof(Part) * note_count);
    int part_count = 0;
    
    for (int i = 0; i < note_count; i++) {
        int assigned = 0;
        for (int p = 0; p < part_count; p++) {
            int can_add = 1;
            for (int n = 0; n < parts[p].note_count; n++) {
                uint32_t overlap_start = notes[i].start_time > parts[p].notes[n].start_time ? 
                                         notes[i].start_time : parts[p].notes[n].start_time;
                uint32_t overlap_end = notes[i].end_time < parts[p].notes[n].end_time ? 
                                       notes[i].end_time : parts[p].notes[n].end_time;
                if (overlap_start < overlap_end) {
                    can_add = 0;
                    break;
                }
            }
            if (can_add) {
                parts[p].notes = realloc(parts[p].notes, sizeof(Note) * (parts[p].note_count + 1));
                parts[p].notes[parts[p].note_count] = notes[i];
                parts[p].note_count++;
                assigned = 1;
                break;
            }
        }
        
        if (!assigned) {
            parts[part_count].notes = malloc(sizeof(Note));
            parts[part_count].notes[0] = notes[i];
            parts[part_count].note_count = 1;
            parts[part_count].total_time = 0;
            part_count++;
        }
    }
    
    // メモリサイズを実際に使用した部分に縮小
    if (part_count > 0 && part_count < note_count) {
        parts = realloc(parts, sizeof(Part) * part_count);
    } else if (part_count == 0) {
        free(parts);
        parts = NULL;
    }
    
    *parts_out = parts;
    *part_count_out = part_count;
}

static char *base_name(const char *path) {
    const char *p = strrchr(path, '/');
    if (!p) p = strrchr(path, '\\');
    p = p ? p + 1 : path;
    char *dst = strdup(p);
    char *dot = strrchr(dst, '.');
    if (dot) *dot = '\0';
    return dst;
}

static void mkdir_p(const char *path) {
    char tmp[1024];
    snprintf(tmp, sizeof(tmp), "%s", path);
    for (char *p = tmp + 1; *p; p++) {
        if (*p == '/' || *p == '\\') {
            *p = '\0';
            mkdir(tmp, 0755);
            *p = '/';
        }
    }
    mkdir(tmp, 0755);
}

// ビッグエンディアンからリトルエンディアンに変換
uint16_t be16_to_le(uint16_t val) {
    return (val >> 8) | (val << 8);
}

uint32_t be32_to_le(uint32_t val) {
    return ((val >> 24) & 0xFF) | ((val >> 8) & 0xFF00) | ((val << 8) & 0xFF0000) | ((val << 24) & 0xFF000000);
}

// デルタタイムを読み込み
uint32_t read_var_len(FILE *fp) {
    uint32_t val = 0;
    uint8_t byte;
    do {
        fread(&byte, 1, 1, fp);
        val = (val << 7) | (byte & 0x7F);
    } while (byte & 0x80);
    return val;
}

// MIDIファイルを解析
MidiFile *parse_midi(const char *filename) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        perror("ファイルを開けません");
        return NULL;
    }

    char chunk_type[5];
    uint32_t chunk_length;

    // ヘッダーチャンク
    fread(chunk_type, 4, 1, fp);
    chunk_type[4] = '\0';
    if (strcmp(chunk_type, "MThd") != 0) {
        fprintf(stderr, "MIDIファイルではありません\n");
        fclose(fp);
        return NULL;
    }

    fread(&chunk_length, 4, 1, fp);
    chunk_length = be32_to_le(chunk_length);

    MidiFile *midi = malloc(sizeof(MidiFile));
    fread(&midi->format, 2, 1, fp);
    midi->format = be16_to_le(midi->format);
    fread(&midi->track_count, 2, 1, fp);
    midi->track_count = be16_to_le(midi->track_count);
    fread(&midi->division, 2, 1, fp);
    midi->division = be16_to_le(midi->division);
    midi->tempo = 120; // デフォルト

    midi->tracks = malloc(sizeof(Track) * midi->track_count);

    // 各トラックを解析
    for (int t = 0; t < midi->track_count; t++) {
        fread(chunk_type, 4, 1, fp);
        if (strcmp(chunk_type, "MTrk") != 0) {
            fprintf(stderr, "トラックチャンクではありません\n");
            free(midi);
            fclose(fp);
            return NULL;
        }

        fread(&chunk_length, 4, 1, fp);
        chunk_length = be32_to_le(chunk_length);

        long track_start = ftell(fp);
        uint32_t current_time = 0;
        uint8_t running_status = 0;
        Note *temp_notes = NULL;
        int note_count = 0;
        midi->tracks[t].parts = NULL;
        midi->tracks[t].part_count = 0;

        while (ftell(fp) < track_start + chunk_length) {
            uint32_t delta_time = read_var_len(fp);
            current_time += delta_time;

            uint8_t status;
            fread(&status, 1, 1, fp);

            if (status < 0x80) {
                // ランニングステータス
                status = running_status;
                fseek(fp, -1, SEEK_CUR);
            } else {
                running_status = status;
            }

            if ((status & 0xF0) == 0x90) { // ノートオン
                uint8_t note, velocity;
                fread(&note, 1, 1, fp);
                fread(&velocity, 1, 1, fp);
                if (velocity > 0) {
                    // ノート開始
                    temp_notes = realloc(temp_notes, sizeof(Note) * (note_count + 1));
                    temp_notes[note_count].note = note;
                    temp_notes[note_count].start_time = current_time;
                    temp_notes[note_count].end_time = 0; // 未設定
                    temp_notes[note_count].velocity = velocity;
                    note_count++;
                } else {
                    // velocity=0 はノートオフ
                    for (int n = note_count - 1; n >= 0; n--) {
                        if (temp_notes[n].note == note && temp_notes[n].end_time == 0) {
                            temp_notes[n].end_time = current_time;
                            break;
                        }
                    }
                }
            } else if ((status & 0xF0) == 0x80) { // ノートオフ
                uint8_t note, velocity;
                fread(&note, 1, 1, fp);
                fread(&velocity, 1, 1, fp);
                // 対応するノートオンを探してend_timeを設定
                for (int n = note_count - 1; n >= 0; n--) {
                    if (temp_notes[n].note == note && temp_notes[n].end_time == 0) {
                        temp_notes[n].end_time = current_time;
                        break;
                    }
                }
            } else if (status == 0xFF) { // メタイベント
                uint8_t type;
                fread(&type, 1, 1, fp);
                uint32_t len = read_var_len(fp);
                
                if (type == 0x51 && len == 3) { // Set Tempo
                    uint8_t b1, b2, b3;
                    fread(&b1, 1, 1, fp);
                    fread(&b2, 1, 1, fp);
                    fread(&b3, 1, 1, fp);
                    uint32_t usec_per_beat = (b1 << 16) | (b2 << 8) | b3;
                    midi->tempo = (uint16_t)(60000000 / usec_per_beat);
                } else {
                    fseek(fp, len, SEEK_CUR);
                }
            } else {
                // その他のMIDIイベントをスキップ
                uint8_t event_type = status >> 4;
                int data_bytes = 0;
                if (event_type == 0x8 || event_type == 0x9 || event_type == 0xA || event_type == 0xB || event_type == 0xE) {
                    data_bytes = 2;
                } else if (event_type == 0xC || event_type == 0xD) {
                    data_bytes = 1;
                } else if (event_type == 0xF) {
                    // システムイベント、簡易スキップ
                    data_bytes = 1;
                }
                fseek(fp, data_bytes, SEEK_CUR);
            }
        }

        // トラックendでend_time未設定のノートをトラックendに設定
        for (int n = 0; n < note_count; n++) {
            if (temp_notes[n].end_time == 0) {
                temp_notes[n].end_time = current_time;
            }
        }

        // タイムディビジョンをミリ秒に変換（簡易的に）
        double ticks_per_beat = midi->division & 0x7FFF;
        double ms_per_tick = 500000.0 / ticks_per_beat / 1000.0; // テンポ120を仮定
        for (int n = 0; n < note_count; n++) {
            temp_notes[n].start_time = (uint32_t)(temp_notes[n].start_time * ms_per_tick);
            temp_notes[n].end_time = (uint32_t)(temp_notes[n].end_time * ms_per_tick);
        }
        
        // 和音を複数パートに分割
        Part *parts;
        int part_count;
        separate_chords(temp_notes, note_count, &parts, &part_count);
        
        // パートの総時間を設定
        for (int p = 0; p < part_count; p++) {
            parts[p].total_time = 0;
            for (int n = 0; n < parts[p].note_count; n++) {
                if (parts[p].notes[n].end_time > parts[p].total_time) {
                    parts[p].total_time = parts[p].notes[n].end_time;
                }
            }
        }
        
        midi->tracks[t].parts = parts;
        midi->tracks[t].part_count = part_count;
        
        free(temp_notes);
    }

    fclose(fp);
    return midi;
}

// トラックを再生（視聴） - シミュレーション
void play_track(Part *part) {
    printf("トラックを視聴（シミュレーション）:\n");
    for (int n = 0; n < part->note_count; n++) {
        Note *note = &part->notes[n];
        uint16_t freq = (uint16_t)midi_note_to_frequency(note->note);
        uint8_t vol = (uint8_t)(note->velocity * 100 / 127);
        uint32_t duration = note->end_time - note->start_time;
        printf("ノート: %d Hz, 音量: %d, 持続: %d ms\n", freq, vol, duration);
        usleep(duration * 1000);
    }
}

// トラックを変換コードとして.txtファイルに出力
void convert_part(Part *part, const char *output_file, int fast_mode) {
    FILE *fp = fopen(output_file, "w");
    if (!fp) {
        perror("Cannot open output file");
        return;
    }

    fprintf(fp, "// SPIKE RT 用変換コード\n");
    fprintf(fp, "#include <spike/hub/speaker.h>\n\n");
    fprintf(fp, "void play_midi() {\n");
    fprintf(fp, "    hub_speaker_set_volume(100); // デフォルト音量\n");
    for (int n = 0; n < part->note_count; n++) {
        Note *note = &part->notes[n];
        uint16_t freq = (uint16_t)midi_note_to_frequency(note->note);
        uint8_t vol = (uint8_t)(note->velocity * 100 / 127);
        uint32_t duration = note->end_time - note->start_time;
        fprintf(fp, "    hub_speaker_set_volume(%d);\n", vol);
        fprintf(fp, "    hub_speaker_play_tone(%d, %d);\n", freq, duration);
    }
    fprintf(fp, "}\n");

    fclose(fp);
}

// 単一パートのHTMLを生成
void write_track_html(const char *html_file, Part *part, uint16_t tempo) {
    FILE *fp = fopen(html_file, "w");
    if (!fp) {
        perror("Cannot open HTML file");
        return;
    }

    fprintf(fp, "<!DOCTYPE html>\n<html lang=\"ja\">\n<head>\n");
    fprintf(fp, "<meta charset=\"utf-8\"><title>SPIKE-RT-MIDI Track Player</title>\n");
    fprintf(fp, "<style>body{font-family:Arial;max-width:900px;margin:50px auto;padding:20px;background-color:#f5f5f5;}\n");
    fprintf(fp, "h1{color:#333;}container{background-color:white;padding:20px;border-radius:8px;box-shadow:0 2px 8px rgba(0,0,0,0.1);}\n");
    fprintf(fp, "button{padding:10px 20px;margin:10px 5px 10px 0;background-color:#667eea;color:white;border:none;border-radius:5px;cursor:pointer;}\n");
    fprintf(fp, "button:hover{background-color:#764ba2;}pre{background-color:#f0f0f0;padding:15px;border-radius:5px;overflow-x:auto;font-size:12px;}\n");
    fprintf(fp, "</style>\n</head>\n<body>\n<div class=\"container\">\n<h1>SPIKE-RT-MIDI Player</h1>\n");
    fprintf(fp, "<button onclick=\"playTrack()\">Play</button>\n<button onclick=\"stopTrack()\">Stop</button>\n");
    fprintf(fp, "<pre id=\"notes\"></pre>\n</div>\n<script>\n");
    
    fprintf(fp, "const notes = [\n");
    for (int n = 0; n < part->note_count; n++) {
        Note *note = &part->notes[n];
        uint16_t freq = (uint16_t)midi_note_to_frequency(note->note);
        uint32_t duration = note->end_time - note->start_time;
        uint8_t vol = (uint8_t)(note->velocity * 100 / 127);
        fprintf(fp, "  {freq:%d,dur:%d,vol:%d,start:%d},\n", freq, duration, vol, note->start_time);
    }
    fprintf(fp, "];\n");
    fprintf(fp, "const tempo = %d;\n", tempo);
    fprintf(fp, "const ctx = new (window.AudioContext || window.webkitAudioContext)();\n");
    fprintf(fp, "let playingOscillators = [];\n");
    fprintf(fp, "function displayNotes() { const html = notes.map((n, i) => 'Note ' + i + ': freq=' + n.freq + 'Hz vol=' + n.vol + '%% dur=' + n.dur + 'ms').join('<br>'); document.getElementById('notes').innerHTML = html; }\n");
    fprintf(fp, "function playTrack() { ctx.resume().then(() => { stopTrack(); let base = ctx.currentTime; notes.forEach(n => { if (n.dur <= 0) return; const o = ctx.createOscillator(); const g = ctx.createGain(); o.connect(g); g.connect(ctx.destination); o.frequency.value = n.freq; o.type = 'sine'; const start = base + n.start / 1000; const end = start + n.dur / 1000; g.gain.setValueAtTime(n.vol / 100 * 0.15, start); g.gain.setValueAtTime(0, end); o.start(start); o.stop(end); playingOscillators.push(o); }); }); }\n");
    fprintf(fp, "function stopTrack() { playingOscillators.forEach(o => { try { o.stop(); } catch (e) { } }); playingOscillators = []; }\n");
    fprintf(fp, "displayNotes();\n");
    fprintf(fp, "</script></body></html>\n");
    
    fclose(fp);
}

void write_all_tracks_html(const char *html_file, Track *tracks, int track_count, uint16_t tempo) {
    FILE *fp = fopen(html_file, "w");
    if (!fp) {
        perror("Cannot open HTML file");
        return;
    }

    fprintf(fp, "<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n<meta charset=\"UTF-8\">\n<title>SPIKE-RT-MIDI Player</title>\n</head>\n<body>\n");
    fprintf(fp, "<h1>SPIKE-RT-MIDI Player</h1>\n");
    fprintf(fp, "<label for=\"trackSelect\">Select Track:</label>\n");
    fprintf(fp, "<select id=\"trackSelect\" onchange=\"updateNotes()\">\n");
    for (int t = 0; t < track_count; t++) {
        int total_notes = 0;
        for (int p = 0; p < tracks[t].part_count; p++) {
            total_notes += tracks[t].parts[p].note_count;
        }
        fprintf(fp, "<option value=\"%d\">Track %d (%d parts, %d notes)</option>\n", t, t, tracks[t].part_count, total_notes);
    }
    fprintf(fp, "</select>\n");
    fprintf(fp, "<button onclick=\"playTrack()\">Play</button><br>\n");
    fprintf(fp, "<pre id=\"notes\"></pre>\n");

    fprintf(fp, "<script>\n");
    fprintf(fp, "const allTracks = [\n");
    for (int t = 0; t < track_count; t++) {
        fprintf(fp, "  [\n");
        for (int p = 0; p < tracks[t].part_count; p++) {
            fprintf(fp, "    [\n");
            for (int n = 0; n < tracks[t].parts[p].note_count; n++) {
                Note *note = &tracks[t].parts[p].notes[n];
                uint16_t freq = (uint16_t)midi_note_to_frequency(note->note);
                uint32_t duration = note->end_time - note->start_time;
                uint8_t vol = (uint8_t)(note->velocity * 100 / 127);
                fprintf(fp, "      {freq:%d,dur:%d,vol:%d,start:%d},\n", freq, duration, vol, note->start_time);
            }
            fprintf(fp, "    ],\n");
        }
        fprintf(fp, "  ],\n");
    }
    fprintf(fp, "];\n");
    fprintf(fp, "const tempo = %d;\n", tempo);
    fprintf(fp, "let currentTrack = 0;\n");
    fprintf(fp, "const ctx = new (window.AudioContext || window.webkitAudioContext)();\n");
    fprintf(fp, "function updateNotes() {\n");
    fprintf(fp, "  currentTrack = document.getElementById('trackSelect').value;\n");
    fprintf(fp, "  const parts = allTracks[currentTrack];\n");
    fprintf(fp, "  let html = '';\n");
    fprintf(fp, "  parts.forEach((part, pi) => { part.forEach((n, i) => { html += 'Part' + pi + ' Note' + i + ': freq:' + n.freq + 'Hz vol:' + n.vol + '%% dur:' + n.dur + 'ms<br>'; }); });\n");
    fprintf(fp, "  document.getElementById('notes').innerHTML = html;\n");
    fprintf(fp, "}\n");
    fprintf(fp, "function playTrack() { ctx.resume().then(() => { const parts = allTracks[currentTrack]; let base = ctx.currentTime; parts.forEach(part => { part.forEach(n => { if (n.dur <= 0) return; const o = ctx.createOscillator(); const g = ctx.createGain(); o.connect(g); g.connect(ctx.destination); o.frequency.value = n.freq; o.type = 'sine'; const start = base + n.start/1000; const end = start + n.dur/1000; g.gain.setValueAtTime(n.vol/100*0.15, start); g.gain.setValueAtTime(0, end); o.start(start); o.stop(end); }); }); }); }\n");
    fprintf(fp, "updateNotes(); // initial\n");
    fprintf(fp, "</script>\n</body>\n</html>\n");

    fclose(fp);
}

void convert_track_to_dir(Track *track, const char *dir, int index, int fast_mode, int overwrite, uint16_t tempo) {
    char track_dir[1024];
    snprintf(track_dir, sizeof(track_dir), "%s/track%d", dir, index);
    mkdir_p(track_dir);

    for (int p = 0; p < track->part_count; p++) {
        if (track->parts[p].note_count == 0) continue;
        
        char part_dir[1024];
        snprintf(part_dir, sizeof(part_dir), "%s/part%d", track_dir, p);
        mkdir_p(part_dir);

        char out_txt[1024];
        char out_html[1024];
        snprintf(out_txt, sizeof(out_txt), "%s/output.txt", part_dir);
        snprintf(out_html, sizeof(out_html), "%s/play.html", part_dir);

        if (!overwrite && !fast_mode) {
            if (access(out_txt, F_OK) == 0 || access(out_html, F_OK) == 0) {
                printf("Overwrite existing files? %s / %s (y/n): ", out_txt, out_html);
                int c = getchar();
                while (getchar() != '\n' && c != EOF) ;
                if (!(c == 'y' || c == 'Y')) {
                    printf("Skipped: %s/track%d/part%d\n", dir, index, p);
                    continue;
                }
            }
        }

        convert_part(&track->parts[p], out_txt, fast_mode);
        write_track_html(out_html, &track->parts[p], tempo);
    }
}

// メイン関数
int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <midi_file> [track_number] [--all] [--fast] [--overwrite|--yes] [--tempo BPM]\n", argv[0]);
        printf("Example: %s song.mid 0\n", argv[0]);
        printf("Example: %s song.mid --all --tempo 140\n", argv[0]);
        return 1;
    }

    const char *midi_path = argv[1];
    int selected = -1;
    int fast = 0;
    int all = 0;
    int overwrite = 0;
    uint16_t custom_tempo = 0;

    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "--all") == 0) {
            all = 1;
        } else if (strcmp(argv[i], "--fast") == 0) {
            fast = 1;
        } else if (strcmp(argv[i], "--overwrite") == 0 || strcmp(argv[i], "--yes") == 0) {
            overwrite = 1;
        } else if (strcmp(argv[i], "--tempo") == 0 && i + 1 < argc) {
            custom_tempo = (uint16_t)atoi(argv[i + 1]);
            i++;
        } else {
            selected = atoi(argv[i]);
        }
    }

    MidiFile *midi = parse_midi(midi_path);
    if (!midi) {
        return 1;
    }

    uint16_t effective_tempo = custom_tempo > 0 ? custom_tempo : midi->tempo;

    printf("Number of tracks: %d\n", midi->track_count);
    printf("Tempo: %d BPM", midi->tempo);
    if (custom_tempo > 0) {
        printf(" (Custom: %d BPM)", effective_tempo);
    }
    printf("\n");
    for (int t = 0; t < midi->track_count; t++) {
        int total_notes = 0;
        for (int p = 0; p < midi->tracks[t].part_count; p++) {
            total_notes += midi->tracks[t].parts[p].note_count;
        }
        printf("Track %d: %d parts, %d total notes\n", t, midi->tracks[t].part_count, total_notes);
    }

    if (!all && selected < 0) {
        if (!fast) {
            printf("視聴したいトラック番号を入力（0-%d）: ", midi->track_count - 1);
            scanf("%d", &selected);
        } else {
            selected = 0;
        }
    }

    if (all) {
        char *base = base_name(midi_path);
        char out_dir[1024];
        snprintf(out_dir, sizeof(out_dir), "%s", base);
        mkdir_p(out_dir);

        for (int t = 0; t < midi->track_count; t++) {
            printf("Converting track %d / %d\n", t+1, midi->track_count);
            convert_track_to_dir(&midi->tracks[t], out_dir, t, fast, overwrite, effective_tempo);
        }

        char all_html[1024];
        snprintf(all_html, sizeof(all_html), "%s/play.html", out_dir);
        write_all_tracks_html(all_html, midi->tracks, midi->track_count, effective_tempo);

        free(base);
    } else {
        if (selected < 0 || selected >= midi->track_count) {
            if (!fast) {
                printf("Enter track number to preview (0-%d): ", midi->track_count - 1);
                scanf("%d", &selected);
            } else {
                selected = 0;
            }
        }

        if (selected < 0 || selected >= midi->track_count) {
            printf("Invalid track number\n");
            for (int t = 0; t < midi->track_count; t++) {
                for (int p = 0; p < midi->tracks[t].part_count; p++) {
                    free(midi->tracks[t].parts[p].notes);
                }
                free(midi->tracks[t].parts);
            }
            free(midi->tracks);
            free(midi);
            return 1;
        }

        if (!fast && !overwrite) {
            printf("Previewing track %d (first part)...\n", selected);
            if (midi->tracks[selected].part_count > 0) {
                play_track(&midi->tracks[selected].parts[0]);
            }

            char confirm;
            printf("Convert this track? (y/n): ");
            scanf(" %c", &confirm);
            if (!(confirm == 'y' || confirm == 'Y')) {
                for (int t = 0; t < midi->track_count; t++) {
                    for (int p = 0; p < midi->tracks[t].part_count; p++) {
                        free(midi->tracks[t].parts[p].notes);
                    }
                    free(midi->tracks[t].parts);
                }
                free(midi->tracks);
                free(midi);
                return 0;
            }
        } else if (fast || overwrite) {
            if (!fast) printf("Auto-overwrite mode: Converting without confirmation\n");
        }

        char *base = base_name(midi_path);
        char out_dir[1024];
        snprintf(out_dir, sizeof(out_dir), "%s", base);
        mkdir_p(out_dir);

        convert_track_to_dir(&midi->tracks[selected], out_dir, selected, fast, overwrite, effective_tempo);

        free(base);
    }

    // メモリ解放
    for (int t = 0; t < midi->track_count; t++) {
        for (int p = 0; p < midi->tracks[t].part_count; p++) {
            free(midi->tracks[t].parts[p].notes);
        }
        free(midi->tracks[t].parts);
    }
    free(midi->tracks);
    free(midi);

    return 0;
}