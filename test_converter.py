#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
SPIKE-RT-MIDI Converter テストスクリプト
"""

import os
import sys

def test_files_exist():
    """ファイルが存在するかテスト"""
    print("🔍 ファイル確認中...")
    required_files = {
        'converter.html': 'メインのウェブサイト',
        'CONVERTER_GUIDE.md': '使用ガイド',
        'SETUP.md': 'セットアップガイド',
        'DREAM.mid': 'テスト用MIDIファイル',
    }
    
    all_exist = True
    for filename, description in required_files.items():
        if os.path.exists(filename):
            size = os.path.getsize(filename)
            print(f"  ✅ {filename:20} ({size:>8} bytes) - {description}")
        else:
            print(f"  ❌ {filename:20} - 見つかりません")
            all_exist = False
    
    return all_exist

def test_html_content():
    """HTMLファイルの内容確認"""
    print("\n📄 HTMLファイル内容確認中...")
    with open('converter.html', 'r', encoding='utf-8') as f:
        content = f.read()
    
    checks = {
        'parseMidi': 'MIDI解析機能',
        'downloadConvertedCode': 'ダウンロード機能',
        'AudioContext': '音声再生機能',
        'JSZip': 'ZIP作成機能',
        'convertAllTracks': 'トラック変換機能',
    }
    
    for keyword, description in checks.items():
        if keyword in content:
            print(f"  ✅ {description}")
        else:
            print(f"  ⚠️  {description} が見つかりません")

def test_midi_file():
    """MIDIファイルの確認"""
    print("\n🎵 MIDIファイル確認中...")
    if os.path.exists('DREAM.mid'):
        size = os.path.getsize('DREAM.mid')
        with open('DREAM.mid', 'rb') as f:
            header = f.read(4)
            if header == b'MThd':
                print(f"  ✅ DREAM.mid は有効なMIDIファイルです ({size} bytes)")
            else:
                print(f"  ❌ DREAM.mid はMIDIファイルではありません")
    else:
        print(f"  ⚠️  DREAM.mid が見つかりません")

def test_pretty_midi():
    """pretty-midiライブラリの確認"""
    print("\n📦 pretty-midiライブラリ確認中...")
    if os.path.exists('pretty-midi'):
        if os.path.exists('pretty-midi/pretty_midi/__init__.py'):
            print(f"  ✅ pretty-midiライブラリが存在します")
        else:
            print(f"  ⚠️  pretty-midiのディレクトリ構造が不完全です")
    else:
        print(f"  ⚠️  pretty-midiディレクトリが見つかりません")

def main():
    print("=" * 60)
    print("  🎵 SPIKE-RT-MIDI Converter テストスイート")
    print("=" * 60)
    
    # Change to script directory
    script_dir = os.path.dirname(os.path.abspath(__file__))
    os.chdir(script_dir)
    
    # Run tests
    files_ok = test_files_exist()
    test_html_content()
    test_midi_file()
    test_pretty_midi()
    
    # Summary
    print("\n" + "=" * 60)
    if files_ok:
        print("✅ 基本的なテストは合格です！")
        print("\n📖 次のステップ:")
        print("  1. SETUP.md を参照してサーバーを起動")
        print("  2. http://localhost:8000/converter.html を開く")
        print("  3. DREAM.mid でテスト実行")
    else:
        print("❌ いくつかのファイルが見つかりません")
        print("📖 SETUP.md を確認してください")
    print("=" * 60)

if __name__ == '__main__':
    main()
