@echo off
REM Double-click launcher for Windows users who don't have a terminal handy.
REM Requires Python 3.8+ installed from https://www.python.org/downloads/
REM (tick "Add python.exe to PATH" during install).

cd /d "%~dp0"

where python >nul 2>nul
if %errorlevel% neq 0 (
    where python3 >nul 2>nul
    if %errorlevel% neq 0 (
        echo Python was not found on PATH.
        echo Install it from https://www.python.org/downloads/ and re-run this file.
        pause
        exit /b 1
    )
    python3 deploy.py
) else (
    python deploy.py
)

echo.
pause
