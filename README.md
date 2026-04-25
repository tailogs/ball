# Bouncing Ball

A fun little Windows utility that creates a bouncing ball on your screen.  
The ball moves with realistic physics, bounces off the edges of the screen, plays a collision sound (optional), and lives in the system tray – perfect for a quick break or as a desktop screensaver alternative.

![screenshot](screenshots/1.jpg)
*Picture 1: Game screenshot*

## Features

- Smooth, frame‑independent movement using high‑precision timers.
- Fully configurable via `config.ini` – change ball radius, speed, color, sound, volume, and more.
- Layered window with alpha blending for a shadow effect.
- System tray icon with a right‑click menu to exit.
- Optional sound on each collision (WAV file support).
- Adjustable volume level (0‑100%) using `waveOutSetVolume`.

## Configuration (`config.ini`)

Create a `config.ini` file in the same directory as the executable. Example:

```bash
ball_radius=30
ball_speed_x=500
ball_speed_y=350
ignore_taskbar=1
enable_sound=1
ball_color=0xFF0000FF
sound_path=sounds\\collision.wav
volume=30
```

| Key             | Description                                           |
|-----------------|-------------------------------------------------------|
| `ball_radius`   | Radius of the ball in pixels.                         |
| `ball_speed_x`  | Horizontal speed (pixels per second).                 |
| `ball_speed_y`  | Vertical speed (pixels per second).                   |
| `ignore_taskbar`| `1` = treat whole screen as bounce area; `0` = exclude taskbar area. |
| `enable_sound`  | `1` = play sound on bounce, `0` = silent.            |
| `ball_color`    | ARGB color in hex (e.g., `0xFFFF0000` = red).         |
| `sound_path`    | Path to the `.wav` file (backslashes escaped).        |
| `volume`        | Volume level from 0 to 100.                           |

## Building

You need **MinGW** (GCC) on Windows (or cross‑compiler on Linux).  
Run:

```bash
make
```

The Makefile links `gdi32`, `user32`, and `winmm`.

## Running

Simply execute `ball.exe`.  
The ball will start bouncing immediately.  
Right‑click the tray icon and select **Exit** to close the program.

## Requirements

- Windows XP / Vista / 7 / 8 / 10 / 11
- Any sound card (if sound enabled)