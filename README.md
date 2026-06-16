# SEdit
Simulation data Editor for Re:sim, multi-agent traffic simulation program.

<img width="960" alt="sedit-image1" src="https://user-images.githubusercontent.com/60654261/81361271-deb70380-9118-11ea-932d-7efaa4afb109.png">

## Platform Support

| Platform | Status |
|----------|--------|
| Windows 10 (Qt5.12 + MinGW64) | ✅ Official |
| macOS (Qt5.15 + Homebrew, Intel x86_64) | ✅ Community port ([PR#1](https://github.com/Reisim/SEdit/pull/1)) |

## Requirements

Dependencies for text rendering and PNG support:
- freetype
- libpng
- zlib
- bzip2

**Windows**: pre-built libraries are in the `libs/` folder.

**macOS** (Homebrew):
```bash
brew install qt@5 freetype libpng zlib bzip2
```

## Build

### Windows
Use Qt5.12 with MinGW64. Library paths are pre-configured in `SEdit.pro`.

### macOS
```bash
mkdir build-SEdit && cd build-SEdit
/usr/local/opt/qt@5/bin/qmake CONFIG+=sdk_no_version_check ../SEdit/src/SEdit.pro
make -j$(sysctl -n hw.logicalcpu)
cp ../SEdit/shaders_fonts/* SEdit.app/Contents/MacOS/
/usr/local/opt/qt@5/bin/macdeployqt SEdit.app
open SEdit.app   # use 'open', not direct binary execution
```

## Usage

The shaders and TrueType font files must be placed in the same folder as the executable (`shaders_fonts/` directory).

**Workflow:**
1. Create a road network by right-clicking the canvas → choose an intersection type
2. Connect nodes via lanes
3. Set traffic volumes in the Scenario Editor
4. Export via `Tool → Show Resim Files Output Dialog` → generates `.rc.txt`, `.rr.txt`, `.rs.txt`, `.ts.txt`
5. Open the exported `.rc.txt` in Re:sim to run the simulation

## Manuals

- Part 1: [Basic Operation](https://github.com/Reisim/SEdit/tree/master/SEdit-Manual-Part1-Basic_Operation.pdf)
- Part 2: [Scenario Data Setting](https://github.com/Reisim/SEdit/tree/master/SEdit-Manual-Part2-Scenario_Data_Setting.pdf)
- Part 3: Advanced Topics — To be described

## Binary

Windows binary (complete set including DLLs): [bin_win10](https://github.com/Reisim/Reisim/tree/master/bin_win10)

## Licence

[LGPL v3](https://github.com/Reisim/Reisim/blob/master/LICENSE)
