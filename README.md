# INF01120 (Software Development) 26/1

[![C++](https://img.shields.io/badge/C++%2026-%2300599C.svg?logo=c%2B%2B&logoColor=white)](https://en.cppreference.com/w/cpp/26)
[![QML](https://img.shields.io/badge/QT%206-41CD52?logo=qt&logoColor=fff)](https://doc.qt.io/qt-6/)
[![Fluidsynth Version](https://img.shields.io/badge/FluidSynth-2.5.4-blue)](https://github.com/FluidSynth/fluidsynth/releases/tag/v2.5.4)
[![libremidi](https://img.shields.io/badge/libremidi-5.4.3-green)](https://github.com/celtera/libremidi)
[![XMake](https://img.shields.io/badge/XMake-v3.0.9-green
)](https://xmake.io/)



A `text ➜ MIDI` tracker. Since it's fun to listen to MIDI, we also support `text ➜ MIDI ➜ audio` via FluidSynth.

## Features

- **Stateful Text Interpreter**: Through character-to-MIDI mapping rules.
- **Multiple MIDI Voices**: Each text line is a new independent voice.
- **FluidSynth Backend**: Real-time MIDI synthesizer integration.
- **Saving and loading text sources** 
- **MIDI Export**: Save generated performances to `.mid` files.

---

## Building the Project

### Dependencies
| Name | Purpose | Managed by XMake? |
| :--- | :--- | :--- |
| **XMake** (v2.8.2+) | Build System | No |
| **C++26 Compiler** | GCC 14+, Clang 18+, or MSVC 2022 | No |
| **Qt6 SDK** | Graphical Interface Toolkit | No |
| **FluidSynth** | Real-time Synthesizer library | **Yes** |


### Linux & macOS
```bash
# Configure XMake to release mode
xmake f -m release
# Build the project
xmake
```

### Windows
See [Windows Guide](docs/README-Windows.md) for toolchain configuration and deployment instructions.

---

## Running the Project

### Runtime Dependencies
- **SoundFont**: A General MIDI `.sf2` or `.sf3` SoundFont file (e.g. `FluidR3_GM.sf2`) is required for audio playback.

```bash
xmake r # or running the executable file directly
```

## Developing

### Development Dependencies
| Tool | Purpose |
| :--- | :--- |
| **clang-format** | Auto-formatting C++ files |
| **clang-tidy** | Static analysis linter |

```bash
xmake format  # Format codebase with clang-format
xmake lint    # Lint codebase with clang-tidy
```

---

## Running Automated Tests

Unit tests are compiled in debug mode using `doctest`:
```bash
xmake f -m debug         # Configure build in debug mode
xmake build test_runner  # Build the tests
xmake run test_runner    # Run the tests
```

## Documentation

Inside [`docs`](./docs) you'll find:
```
docs # This project's actual documentation
├── spec # Pimenta's provided requirement files
│   ├── Phase1.pdf
│   ├── Phase2.pdf
│   ├── Phase3.pdf
│   ├── Test_suggestions.md
│   └── Test_suggestions.pdf
└── reports # Our reports
    ├── Phase1.pdf
    ├── Phase2.pdf
    ├── Phase3.pdf
    └── src # The source files for the reports
```
