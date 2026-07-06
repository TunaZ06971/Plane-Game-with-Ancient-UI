# Plane Fight with Ancient UI

A retro terminal-based vertical shooter written in C++14 with `ncurses`.

This project was developed as my HKU COMP2113 course assignment in Fall 2025 and received full marks. The game uses a fixed 50 x 40 logical playfield, ASCII sprites, colored terminal rendering, randomized falling obstacles, auto-shooting, per-difficulty score files, and a final boss encounter.

## Demo

[Watch the gameplay demo on YouTube](https://www.youtube.com/watch?v=AC8ug2uTMoU)

## Gameplay Overview

The player controls a small ASCII plane near the bottom of the terminal window. Enemy blocks fall from the top, each with different behavior:

- `SOLID` blocks stop bullets and damage the player on collision.
- `BREAKABLE` blocks have HP and award score when destroyed.
- `EXPLODE` blocks trigger area damage after being destroyed.
- `DAMAGE_BUFF` blocks can increase the player's bullet damage when collected after being upgraded.

After surviving long enough, normal waves stop and a large ASCII boss descends into the arena. The boss has difficulty-scaled HP and fires patterned bullets. Defeating the boss leads to the win screen; running out of lives leads to game over.

## Features

- Real-time terminal rendering with `ncurses`
- Centered 50 x 40 game board with colored ASCII UI
- Start menu, difficulty selection, gameplay, win, game-over, and stats screens
- Easy, Medium, and Hard difficulty configurations
- Randomized enemy waves, spawn positions, obstacle types, speed, and values
- Auto-shoot toggle with difficulty-dependent cooldown
- Player lives, score, kill count, and bullet damage tracking
- Boss fight with spawn animation, HP, collision checks, and bullet patterns
- Persistent per-difficulty leaderboards stored in text files
- Modular C++ source layout using classes, enums, STL containers, and smart pointers

## Build and Run

### Prerequisites

Use a Unix-like terminal environment such as Linux, macOS, WSL, or HKU CS servers.

Required tools:

- `g++` with C++14 support
- `make`
- `ncurses` development library

On Debian/Ubuntu/WSL:

```bash
sudo apt-get update
sudo apt-get install build-essential libncurses-dev
```

On macOS, `ncurses` is usually available through the system toolchain. If needed:

```bash
brew install ncurses
```

### Compile

```bash
make
```

### Run

```bash
./plane_game
```

Run the binary from the project root so the game can read and write:

- `stats_easy.txt`
- `stats_medium.txt`
- `stats_hard.txt`

### Clean Build Artifacts

```bash
make clean
```

### Manual Compilation

If `make` is unavailable:

```bash
g++ -std=c++14 -Wall -Wextra -O2 \
    main.cpp Game.cpp GameObject.cpp Player.cpp Bullet.cpp Enemy.cpp Display.cpp Boss.cpp \
    -lncurses -o plane_game
```

## Controls

### Menus

| Key | Action |
| --- | --- |
| `W` / `A` / `S` / `D` | Move selection |
| Arrow keys | Move selection |
| `SPACE` / `ENTER` | Confirm |
| `Q` | Quit from some menu screens |

### In Game

| Key | Action |
| --- | --- |
| `W` / `A` / `S` / `D` | Move the plane |
| Arrow keys | Move the plane |
| `SPACE` | Toggle auto-shoot |
| `Q` | Return to the main menu |

### End and Stats Screens

| Key | Action |
| --- | --- |
| `X` | Open the stats screen after win/game over |
| `A` / `D` or arrow keys | Switch between stats buttons |
| `SPACE` / `ENTER` | Select `BACK` or `CLEAN` |

## Difficulty Design

| Difficulty | Player Lives | Spawn Pace | Max Enemies | Enemy Wave Size | Boss Timing |
| --- | ---: | --- | ---: | --- | --- |
| Easy | 5 | Slow | 15 | 2-4 | About 20 seconds |
| Medium | 4 | Medium | 25 | 4-6 | About 30 seconds |
| Hard | 3 | Fast | 40 | 6-8 | About 30 seconds |

Difficulty also changes enemy speed, obstacle HP, score per kill, auto-shoot cooldown, boss HP, and boss firing frequency.

## Project Structure

```text
.
├── main.cpp              # Program entry point
├── Game.h / Game.cpp     # State machine, gameplay loop, spawning, collisions, scoring, stats I/O
├── Display.h / Display.cpp
│                         # ncurses setup, layout, menus, HUD, stats, end screens
├── GameObject.h / GameObject.cpp
│                         # Base class for position-based entities
├── Player.h / Player.cpp # Player movement and ASCII plane rendering
├── Enemy.h / Enemy.cpp   # Falling obstacle types, HP, values, movement, rendering
├── Bullet.h / Bullet.cpp # Player and boss bullets with ownership and velocity
├── Boss.h / Boss.cpp     # Boss HP, spawn animation, bullet patterns, collision logic
├── GameTypes.h           # Shared enums, constants, difficulty config, coordinate helpers
├── Makefile              # Build rules
├── stats_easy.txt        # Easy-mode leaderboard storage
├── stats_medium.txt      # Medium-mode leaderboard storage
└── stats_hard.txt        # Hard-mode leaderboard storage
```

## COMP2113 Requirement Mapping

The project was designed around the common COMP2113 programming requirements.

### 1. Random Game Events

Randomness is used for:

- Enemy spawn positions
- Enemy wave size
- Spawn intervals
- Obstacle type selection
- Enemy speed selection
- Boss bullet patterns

The random seed is initialized in the `Game` constructor with `srand(time(nullptr))`.

### 2. Data Structures for Game Status

The game uses standard C++ containers and enums to manage state:

- `std::vector<std::unique_ptr<Bullet>> bullets`
- `std::vector<std::unique_ptr<Enemy>> enemies`
- `std::vector<int> leaderboard`
- `enum class GameState`
- `enum class Difficulty`
- `enum class ObstacleType`
- `DifficultyConfig`

These structures track active entities, player progress, current screen state, selected difficulty, and persistent scores.

### 3. Dynamic Memory Management

Dynamic entities are managed with RAII and smart pointers:

- Bullets and enemies are created with `std::make_unique`.
- The boss is created only when the boss phase begins.
- `std::unique_ptr` automatically releases memory when objects are removed or reset.

This avoids manual `new`/`delete` and keeps ownership clear.

### 4. File Input/Output

Leaderboards are saved with standard file streams:

- `std::ifstream` loads score history.
- `std::ofstream` saves sorted scores.
- `clearGameStats` truncates the selected difficulty's score file.

Each difficulty has a separate leaderboard file.

### 5. Multiple Source Files

The implementation is split into multiple `.h` and `.cpp` files by responsibility: game control, rendering, shared types, player, enemies, bullets, boss, and base game objects.

## Technical Notes

- Language standard: C++14
- Rendering/input library: `ncurses`
- Game loop timing: `usleep(50000)`, roughly 20 FPS
- Logical board size: 50 x 40
- Build target: `plane_game`
- Persistent data format: one integer score per line in each `stats_*.txt` file

## Repository Notes

The included `.gitignore` excludes local build artifacts such as object files and the compiled `plane_game` binary. Source files, the Makefile, README, and score text files are intended to remain version-controlled.
