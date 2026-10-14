<p align="center">
  <img src="src/icon.png" width="84" alt="Hollow Knight icon">
</p>

<h1 align="center">Hollow Knight (WebAssembly / Web Port)</h1>

<p align="center">
  <b>High-quality WebAssembly implementation of Hollow Knight in the browser.</b><br>
  Explore Hallownest as the Knight from King's Pass to Hornet in Greenpath, now running natively on the web.
</p>

## Overview

This project provides a clean, fast WebAssembly/WASM target for playing Hollow Knight in web browsers. It builds on the efficient C engine while leveraging modern web technologies (HTML5 Canvas 2D, Web Audio API, and reconfigurable keyboard controls) for a sleek, minimal, high-fidelity experience without severe calculator downscaling constraints.

## Web Features

* **Minimal Sleek Interface:** Dark ambient UI shell with fullscreen toggle, customizable keybinding modal, and clean viewport.
* **High-Quality Web Graphics:** Re-packs game assets in full resolution (`HK_WEB_QUALITY=1`), removing calculator tile budget limits and half-height rendering skips (`g_gfx_fast`).
* **Customizable Controls:** Interactive web keybinding modal supporting full control remapping saved to browser local storage.
* **Persistent Saves:** Game progress automatically saved to browser storage via Emscripten IDBFS.
* **Web Audio Bridge:** Web Audio API integration for web sound support.

## Building for the Web

### Prerequisites

* Python 3 and NumPy / Pillow
* Emscripten SDK (`emcc`)
* A legally owned copy of Hollow Knight (Linux/Mac/Windows installation Data directory)

> **Note on Emscripten Installation:**
> If installing Emscripten from your system package manager (e.g., Arch Linux) feels unnecessarily large, you can easily use the official lightweight **emsdk**:
> ```bash
> git clone https://github.com/emscripten-core/emsdk.git
> cd emsdk
> ./emsdk install latest
> ./emsdk activate latest
> source ./emsdk_env.sh
> ```
> Or run the build inside a minimal Emscripten Docker container (`emscripten/emsdk`).

### Build Steps

1. **Extract and Pack High-Quality Game Assets:**

Set `HOLLOWKNIGHT` to your local Hollow Knight Data directory (for example, Steam's installation directory ending in `hollow_knight_Data`):

```bash
make data HOLLOWKNIGHT="$HOME/.local/share/Steam/steamapps/common/Hollow Knight/hollow_knight_Data"
```

This runs the asset extraction pipeline with `HK_WEB_QUALITY=1` to build `src/data.bin` without heavy calculator downscaling.

2. **Compile the WebAssembly Bundle:**

```bash
make web
```

This compiles the game into `build/web/hollowknight.html` (alongside `.js` and `.wasm` files) using the custom minimal HTML shell (`shell.html`).

3. **Run the Game:**

Serve the `build/web/` folder with any static HTTP server:

```bash
python3 -m http.server 8080 --directory build/web
```

Open `http://localhost:8080/hollowknight.html` in your browser.

## Controls & Keybindings

Default web controls:

| Action | Default Key |
| --- | --- |
| Move Left / Right | Left Arrow / Right Arrow |
| Look Up / Down | Up Arrow / Down Arrow |
| Jump / Confirm | Space |
| Attack / Cancel | X |
| Focus / Spell | C or Z |
| Dash | Shift |
| Inventory | I |
| Quick Map | Tab |
| Pause Menu | Backspace |
| Title / Quit | Escape |

Controls can be fully rebound in real time by clicking the **Keybinds** button in the top navigation bar.

## Calculator Target

The ARM calculator target (`hollowknight.nwa`) remains available in `Makefile` (`make build`) for NumWorks devices.

## License

The code is licensed under the GNU General Public License v3.0 (see `LICENSE`). Hollow Knight assets, artwork, maps, and fonts are property of Team Cherry and are extracted locally from the user's legal game files.
