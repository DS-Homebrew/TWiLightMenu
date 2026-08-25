#!/usr/bin/env bash
cd "$(dirname "$0")"
if command -v xdg-open >/dev/null; then xdg-open index.html
elif command -v open >/dev/null; then open index.html
elif command -v start >/dev/null; then start index.html
else echo "Abra sender/index.html no navegador manualmente."; fi
