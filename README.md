# INF01120 (Software Development) 26/1

[![C++](https://img.shields.io/badge/C++%2026-%2300599C.svg?logo=c%2B%2B&logoColor=white)](https://en.cppreference.com/w/cpp/26)
[![QML](https://img.shields.io/badge/QT%206-41CD52?logo=qt&logoColor=fff)](https://doc.qt.io/qt-6/)
[![Fluidsynth Version](https://img.shields.io/badge/FluidSynth-2.5.4-blue)](https://github.com/FluidSynth/fluidsynth/releases/tag/v2.5.4)
[![libremidi](https://img.shields.io/badge/libremidi-5.4.3-green)](https://github.com/celtera/libremidi)
[![XMake](https://img.shields.io/badge/XMake-v3.0.9-green
)](https://xmake.io/)



A `text ➜ MIDI` tracker. Since it's fun to listen to MIDI, we also support `text ➜ MIDI ➜ audio` via FluidSynth.

---

## Features

- **Stateful Text Interpreter**: Through character-to-MIDI mapping rules.
- **Multiple MIDI Voices**: Each text line is a new independent voice.
- **FluidSynth Backend**: Real-time MIDI synthesizer integration.
- **Saving and loading text sources** 
- **MIDI Export**: Save generated performances to `.mid` files.

## Prerequisites

- **Build System**: XMake
- **Dependencies**: 
  - **Qt6** (Core, Widgets)
  - **FluidSynth** 
  - **libremidi**

## Building the Project

For detailed instructions on setting up prerequisites and compiling the application on Windows, see the [Windows Guide](docs/README-Windows.md).

## Running Tests

Unit tests will probably be managed through CTest?

## Documentation

Inside [`docs`]() you'll find:
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
    └── src # The source files for the reports
```
