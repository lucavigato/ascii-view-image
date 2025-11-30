# ascii-view (Enhanced Fork)

**ascii-view** is a command-line tool written in C that converts images into ASCII art. 

Originally designed to simply print colored characters to the terminal, this fork transforms it into a powerful **ASCII Art Image Generator**. You can now export high-resolution PNGs, create "Pixel Art" style upscales, and control the exact output dimensions, all while keeping the ability to view the result in your console.

## Features

*   **Console Preview**: View images as colored ASCII text directly in your terminal.
*   **High-Quality Export**: Save your ASCII art as **PNG** images with high-quality text rendering (powered by Cairo & Pango).
*   **Pixel Art Scaling (`--scale`)**: Turn any image into a detailed ASCII mosaic. A scale of 10 means each original pixel becomes a 10x10 block containing a character.
*   **Target Resolution (`--dims`)**: Force the output image to be exactly 1920x1080 (or any other size), automatically adjusting the grid density.
*   **Retro Mode**: Optional 3-bit color palette (8 colors) for a vintage terminal look.
*   **Edge Detection**: Uses Sobel filters to detect edges and use directional characters (`|`, `/`, `-`, `\`) for better shapes.

## Prerequisites

To build the project, you need a C compiler and the Pango/Cairo development libraries.

**Linux (Debian/Ubuntu):**
```bash
sudo apt update
sudo apt install build-essential pkg-config libcairo2-dev libpango1.0-dev
```

**Linux (Fedora):**
```bash
sudo dnf install gcc make pkgconf-pkg-config cairo-devel pango-devel
```

## Building

Simply run `make` in the project root:

```bash
make
```
This will produce the `ascii-view` executable.

## Usage

### 1. Basic Terminal View
Prints the converted image to standard output.
```bash
./ascii-view images/photo.jpg
```

### 2. Export to Image (`--export`)
Saves the output to a PNG file.
```bash
./ascii-view images/photo.jpg --export --output result.png
```

### 3. Scaling / Zoom (`--scale`)
Great for creating detailed digital art.
*   `--scale 1`: 1 pixel = 1 character (1x1 render). Keeps original resolution.
*   `--scale 10`: 1 pixel = 1 character (rendered as 10x10 block). Image size increases by 10x (like a nearest-neighbor zoom).

```bash
./ascii-view images/icon.png --scale 10 -o icon_ascii.png
```

### 4. Fixed Resolution / Wallpaper (`--dims`)
Forces the output image to a specific resolution. The tool calculates the optimal grid density to fit.
```bash
./ascii-view images/wallpaper.jpg --dims 1920x1080 -o wallpaper_hd.png
```

### 5. Retro Aesthetic
Use `--retro-colors` to snap colors to a basic 8-color palette.
```bash
./ascii-view images/photo.jpg --retro-colors -e -o retro.png
```

## Options Reference

| Flag | Description |
| :--- | :--- |
| `-w`, `--width <n>` | Set specific width in characters for terminal output. |
| `-e`, `--export` | Save to file instead of printing to terminal. |
| `-o`, `--output <file>` | Specify output filename (default: `input_ascii.png`). |
| `-s`, `--scale <n>` | **Pixel Replacement Mode**: 1 char replaces an NxN block of pixels. |
| `--dims <WxH>` | **Target Resolution Mode**: Force output to specific pixel dimensions. |
| `--retro-colors` | Use 3-bit color palette (8 colors). |
| `--font <name>` | Specify font family for export (default: "DejaVu Sans Mono"). |
| `--bg-white` | Use white background instead of black. |

## Credits

Based on the original work by [Gouws Xander](https://github.com/gouwsxander/ascii-view).
Enhanced and refactored by Luca.
