# Native Windows Sandbox Rendering Engine

A pure native Windows application optimized for low-tier laptops. No web technologies, no Electron, no Chromium - just pure Win32 API and GDI.

## Features

- **Pure Native Code**: 100% Win32 API + GDI rendering
- **Optimized Performance**: Spatial hashing for O(n) collision detection
- **Physics Simulation**: Full rigid body physics with collisions, gravity, and restitution
- **Particle System**: Free-list reuse for efficient particle management
- **Multiple Tools**: Draw, Move, Delete, Particles, and Force tools
- **Shape Types**: Circle, Rectangle, Triangle, and Line
- **Save/Load Scenes**: JSON-like format for persisting simulations
- **Low Power Mode**: Doubles timestep for better performance on weak hardware
- **GDI Object Caching**: Prevents handle leaks and improves rendering speed

## Architecture

This engine implements the architectural patterns from the Rust/C# Godot reference:

- **Spatial Hash Grid**: O(log n) lookup similar to BSP trees
- **Lazy Instantiation**: Objects only rendered when active
- **Data-Oriented Design**: Separate pools for objects and particles
- **Cache-Conscious**: GDI object caching to prevent kernel handle exhaustion

## Building

### Requirements
- Windows 7 or later
- MinGW-w64 (GCC) OR Microsoft Visual C++

### Quick Build
```batch
build.bat
```

### Manual Build (MinGW)
```batch
gcc -O2 -o sandbox_engine.exe sandbox_engine.c -lgdi32 -luser32 -lcomctl32 -lm -mwindows
```

### Manual Build (MSVC)
```batch
cl /O2 /Fe:sandbox_engine.exe sandbox_engine.c gdi32.lib user32.lib comctl32.lib
```

## Controls

| Key/Action | Function |
|------------|----------|
| **1** | Draw Tool |
| **2** | Move Tool |
| **3** | Delete Tool |
| **4** | Particle Tool |
| **5** | Force Tool (radial blast) |
| **C** | Circle Shape |
| **R** | Rectangle Shape |
| **T** | Triangle Shape |
| **L** | Line Shape |
| **S** | Save Scene |
| **O** | Load Scene |
| **P** | Toggle Physics |
| **M** | Toggle Low Power Mode |
| **Left Click** | Use Current Tool |
| **Right Click** | Spawn Particles |

## Bug Fixes Applied (v3)

All critical bugs from the code review have been fixed:

1. ✅ **gridNext field added** - Separated spatial hash linked list from mass field (was corrupting physics)
2. ✅ **particleCount high-water mark** - No longer decremented, prevents skipping active particles
3. ✅ **SaveScene/LoadScene format** - Consistent %f for all floats, fixed newline escape
4. ✅ **GetClientRect optimization** - Moved outside particle loop (was syscall per particle)
5. ✅ **freeParticleList bounds check** - Prevents buffer overflow
6. ✅ **GDI cache unified** - One entry per color with both brush and pen
7. ✅ **gridNext initialization** - Properly initialized in InitObjects and LoadScene

## Performance Characteristics

- **Memory**: ~2-5 MB RAM depending on object count
- **CPU**: Single-threaded, optimized for single-core performance
- **Rendering**: Double-buffered GDI with BitBlt
- **Collision Detection**: O(n) average case with spatial hashing
- **Max Objects**: 500 (configurable via MAX_OBJECTS)
- **Max Particles**: 2000 (configurable via MAX_PARTICLES)

## License

Public Domain / MIT - Use freely for any purpose.
