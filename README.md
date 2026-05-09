# Sandbox Rendering Engine - Native Windows Application

A **pure native Windows application** built with Win32 API and GDI. No web technologies, no Electron, no Chromium wrapper.

## Features

- **Pure Native Code**: Written in C using only Win32 API and GDI
- **Optimized for Low-Tier Hardware**: Efficient rendering with backbuffer, adjustable performance modes
- **Full Physics Simulation**: Gravity, collisions, momentum, restitution
- **Multiple Shape Types**: Circle, Rectangle, Triangle, Line
- **Particle System**: Up to 2000 particles with physics
- **Tool Modes**: Draw, Move, Delete, Particles, Physics (force field)
- **Scene Management**: Save and load scenes to/from text files
- **Color Picker**: Right-click to choose colors
- **Real-time Stats**: FPS counter, object count, particle count

## Requirements

### To Build:
- **MinGW-w64** (GCC for Windows) OR **MSYS2**
- Windows SDK (usually comes with Visual Studio or MinGW)

### To Run (compiled executable):
- Windows 7 or later
- No additional dependencies required!

## Installation

### Option 1: MSYS2 (Recommended)
1. Download MSYS2 from https://www.msys2.org/
2. Install to default location
3. Open MSYS2 MinGW x64 terminal
4. Install toolchain: `pacman -S mingw-w64-x86_64-toolchain`
5. Navigate to this directory
6. Run: `gcc -O2 -o sandbox_engine.exe sandbox_engine.c -lgdi32 -luser32 -lcomctl32 -lm -mwindows`

### Option 2: MinGW-w64 Standalone
1. Download from https://www.mingw-w64.org/
2. Extract and add `bin` folder to PATH
3. Run `build.bat` or compile manually

### Option 3: Visual Studio
1. Create a new Win32 Project
2. Replace the generated code with `sandbox_engine.c`
3. Add `comctl32.lib` to linker dependencies
4. Build

## Building

### Using the build script:
```batch
build.bat
```

### Manual compilation:
```batch
gcc -O2 -o sandbox_engine.exe sandbox_engine.c -lgdi32 -luser32 -lcomctl32 -lm -mwindows
```

## Usage

### Controls:
- **Left Click**: Use the selected tool
- **Right Click**: Open color picker
- **Tools** (select via radio buttons):
  - **Draw**: Create new objects
  - **Move**: Drag objects around
  - **Delete**: Remove objects
  - **Particles**: Spawn particle effects
  - **Physics**: Apply force field to push/pull objects

### Shapes:
- Circle
- Rectangle
- Triangle
- Line

### Options:
- **Low Power Mode**: Reduces physics update rate for better performance on very old hardware
- **Clear All**: Remove all objects and particles
- **Save**: Export scene to text file
- **Load**: Import scene from text file

## Technical Details

### Architecture:
- **Window Procedure**: Main message loop handling WM_CREATE, WM_TIMER, WM_COMMAND
- **Custom Canvas**: Subclassed static control for rendering area
- **Double Buffering**: Back buffer DC prevents flickering
- **Physics Engine**: Custom impulse-based collision resolution
- **Particle System**: Simple lifecycle-based particle management

### Performance Optimizations:
- Fixed-size arrays (no dynamic allocation during runtime)
- Spatial hashing ready (currently O(n²) collision detection, can be optimized)
- Configurable timer interval for low-power mode
- GDI batch rendering with minimal state changes

### Memory Usage:
- ~100 KB base memory footprint
- Objects: 500 max × 48 bytes = 24 KB
- Particles: 2000 max × 32 bytes = 64 KB
- Total: <200 KB typical usage

## File Format

Scene files are plain text:
```
<object_count>
<shape> <x> <y> <vx> <vy> <radius> <width> <height> <color> <mass> <restitution>
...
```

## License

Public domain / MIT - Use freely for any purpose.

## Troubleshooting

### "GCC not found"
- Ensure MinGW is installed and `gcc.exe` is in your PATH
- Test by running `gcc --version` in command prompt

### Compilation errors
- Make sure you're using a MinGW compiler, not MSVC
- For MSVC, create a Win32 project and adjust includes

### Poor performance
- Enable "Low Power" mode in the toolbar
- Reduce the number of objects/particles
- Close other applications

## Future Enhancements

Potential additions for extended functionality:
- Texture mapping support
- More shape types (polygons, curves)
- Joints/constraints for ragdoll physics
- Scripting interface (Lua)
- Multi-threaded physics
- OpenGL/DirectX renderer option

---

**This is a 100% native Windows application with zero web technology dependencies.**
