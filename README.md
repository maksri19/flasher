# Project Name

A brief, one-sentence description of what your C/OpenGL application does.

![Screenshot of the application](path/to/screenshot.png)

## Features
* Feature 1 (e.g., Renders 3D models / visualizes directory structures)
* Feature 2 (e.g., Cross-platform support for Linux and Windows)
* Feature 3 (e.g., Lightweight, written in pure C with minimal dependencies)

## Quick Start (Pre-compiled Binaries)
1. Go to the [Releases](../../releases) page and download the latest archive for your OS.
2. **Windows:** Extract the `.zip` file and double-click `your_program.exe`.
3. **Linux:** Extract the archive, make the file executable, and run it:
   ```bash
   chmod +x your_program
   ./your_program
   ```
*(Note: Requires a GPU and drivers supporting OpenGL 3.3 or newer.)*

## Building from Source

### 1. Install Dependencies
**Ubuntu / Debian:**
```bash
sudo apt update
sudo apt install build-essential cmake libglfw3-dev libgl1-mesa-dev
```

**Fedora:**
```bash
sudo dnf install gcc cmake glfw-devel mesa-libGL-devel
```

**Windows (via MSYS2 / MinGW-w64):**
Open the **MSYS2 MinGW 64-bit** terminal and run:
```bash
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake mingw-w64-x86_64-glfw
```

### 2. Compile
```bash
git clone https://github.com
cd your-repo
# Replace this with your actual gcc, make, or cmake commands:
gcc src/*.c -o your_program -lglfw -lGL -lm -ldl
```

## Controls & Usage

| Key / Input | Action |
| :--- | :--- |
| `W` `A` `S` `D` | Move / Navigate |
| `Mouse` | Look / Interact |
| `Esc` | Close window |

## Built With
* [GLFW](https://glfw.org) - Window and input handling
* [GLAD](https://dav1d.de) - OpenGL function loader

## License
This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

