#!/usr/bin/env bash
# Double-click launcher for macOS users who don't have a terminal handy.
# (macOS may ask you to confirm running it once, under System Settings > Privacy & Security.)
cd "$(dirname "$0")" || exit 1
python3 deploy.py
echo
read -r -p "Press Enter to close this window..." _
