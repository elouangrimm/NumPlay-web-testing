<p align="center">
  <img src="docs/media/icon.png" width="96" alt="NumPlay icon">
</p>

<h1 align="center">NumPlay (Web & WebAssembly Games)</h1>

<p align="center">
  <b>High-quality WebAssembly ports of classic games, featuring Hollow Knight on the web.</b>
</p>

## Highlights: Hollow Knight Web Target

Experience Hollow Knight running natively in your web browser via WebAssembly (`WASM`):

* **Sleek Dark Interface:** Clean dark UI with fullscreen mode and full customizable keybindings.
* **Full-Quality Web Rendering:** High resolution texture packing (`HK_WEB_QUALITY=1`) with full frame rate and smooth rendering.
* **Persistent Saves:** Saved game state kept safely in browser storage (`IDBFS`).
* **Legal Asset Pipeline:** Uses your locally owned copy of Hollow Knight (`hollow_knight_Data`) to extract and pack high quality game assets without copyright redistribution.

For detailed compilation and setup instructions, see [games/hollowknight/README.md](games/hollowknight/README.md).

### Quick Web Build Commands

```bash
cd games/hollowknight
make data HOLLOWKNIGHT="/path/to/hollow_knight_Data"
make web
python3 -m http.server 8080 --directory build/web
```

Open `http://localhost:8080/hollowknight.html` to play!

## License

Code licensed under the GNU General Public License v3.0 (see `LICENSE`). Game assets belong to their respective creators.
