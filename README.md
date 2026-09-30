# Rotation Matrix Point Calculator

A Windows desktop app in C++ and OpenGL. You enter a 3D point, an angle, and an axis (X, Y, or Z). The program computes the rotated point **P′** and shows both points on the X/Y/Z axes.

**Author:** Tal Halbanny

## Features

- 3D view of the X, Y, and Z axes
- Editable starting point **P** and rotation angle
- Axis choice with X / Y / Z buttons
- **Calculate Point** shows **P′** in the panel and as a gold marker
- The original point stays visible in cyan for comparison
- Reset returns to the default values

## How to run

Windows 10 or 11. No extra installs if you use the built exe.

```bat
build.bat
build\PointRotation.exe
```

Or open `build\PointRotation.exe` after it has been compiled.

## How to use

1. Enter the point in **Point P (x, y, z)**, or keep `1.2, 0.7, 0.4`.
2. Enter the angle in degrees.
3. Press **X**, **Y**, or **Z**.
4. Press **Calculate Point**.
5. **P′** appears under Calculated Point (yellow in the 3D view; original **P** is blue/cyan).
6. Press **Reset** to restore the defaults.

## Build from source

Needs a C++17 compiler (LLVM MinGW or MinGW-w64) on Windows.

```bat
build.bat
```

Or with CMake:

```bat
cmake -B build
cmake --build build
```

The app links Windows OpenGL: `opengl32`, `glu32`, `user32`, `gdi32`.
