/* Web platform: browser keyboard, dynamic keybindings, Web Audio bridge, framebuffer output and save files. */
#include "hk.h"
#include <emscripten.h>
#include <emscripten/html5.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

uint16_t web_fb[SCREEN_W * SCREEN_H];

EM_JS(void, web_canvas_init, (), {
  var c = Module["canvas"];
  if (!c) {
    c = document.createElement("canvas");
    document.body.appendChild(c);
    Module["canvas"] = c;
  }
  c.width = 320;
  c.height = 240;
  c.style.width = "960px";
  c.style.height = "720px";
  c.style.maxWidth = "100%";
  c.style.maxHeight = "100%";
  c.style.imageRendering = "pixelated";
  Module.hkCtx = c.getContext("2d", {alpha: false});
  Module.hkImage = Module.hkCtx.createImageData(320, 240);

  // Web Audio Context setup
  try {
    var AudioCtx = window.AudioContext || window.webkitAudioContext;
    if (AudioCtx) {
      Module.hkAudioCtx = new AudioCtx();
    }
  } catch (e) {
    console.warn("Web Audio API not supported", e);
  }

  // Key map storage on Module
  Module.hkKeyMap = {
    "ArrowLeft": 1,        // K_LEFT
    "ArrowRight": 2,       // K_RIGHT
    "ArrowUp": 4,          // K_UP
    "ArrowDown": 8,        // K_DOWN
    "Space": 16 | 512,     // K_JUMP | K_OK
    "Enter": 512,          // K_OK
    "KeyX": 32 | 1024,     // K_ATTACK | K_BACK
    "ShiftLeft": 256,      // K_DASH
    "ShiftRight": 256,     // K_DASH
    "KeyC": 64 | 128,      // K_FOCUS | K_SPELL
    "KeyZ": 64 | 128,      // K_FOCUS | K_SPELL
    "KeyI": 8192,          // K_INV
    "Tab": 4096,           // K_MAP
    "Backspace": 2048,     // K_PAUSE
    "Escape": 16384        // K_HOME
  };

  Module.updateKeyMap = function(newMap) {
    if (newMap && typeof newMap === "object") {
      Module.hkKeyMap = newMap;
    }
  };
});

EM_JS(uint32_t, web_get_key_code, (const char *code_str), {
  var code = UTF8ToString(code_str);
  if (!code) return 0;
  if (Module.hkKeyMap && Module.hkKeyMap[code] !== undefined) {
    return Module.hkKeyMap[code];
  }
  return 0;
});

EM_JS(void, web_play_sound_event, (const char *evt_str), {
  if (!Module.hkAudioCtx) {
    try {
      var AudioCtx = window.AudioContext || window.webkitAudioContext;
      if (AudioCtx) Module.hkAudioCtx = new AudioCtx();
    } catch(e) {}
  }
  if (!Module.hkAudioCtx) return;
  if (Module.hkAudioCtx.state === "suspended") {
    Module.hkAudioCtx.resume();
  }
  var ctx = Module.hkAudioCtx;
  var now = ctx.currentTime;
  var evt = UTF8ToString(evt_str);

  if (evt === "jump") {
    var osc = ctx.createOscillator();
    var gain = ctx.createGain();
    osc.type = "sine";
    osc.frequency.setValueAtTime(150, now);
    osc.frequency.exponentialRampToValueAtTime(380, now + 0.12);
    gain.gain.setValueAtTime(0.2, now);
    gain.gain.exponentialRampToValueAtTime(0.01, now + 0.12);
    osc.connect(gain);
    gain.connect(ctx.destination);
    osc.start(now);
    osc.stop(now + 0.12);
  } else if (evt === "dash") {
    var bufferSize = Math.floor(ctx.sampleRate * 0.15);
    var buffer = ctx.createBuffer(1, bufferSize, ctx.sampleRate);
    var data = buffer.getChannelData(0);
    for (var i = 0; i < bufferSize; i++) data[i] = Math.random() * 2 - 1;
    var noise = ctx.createBufferSource();
    noise.buffer = buffer;
    var filter = ctx.createBiquadFilter();
    filter.type = "bandpass";
    filter.frequency.setValueAtTime(1200, now);
    filter.frequency.exponentialRampToValueAtTime(300, now + 0.15);
    var gain = ctx.createGain();
    gain.gain.setValueAtTime(0.25, now);
    gain.gain.exponentialRampToValueAtTime(0.01, now + 0.15);
    noise.connect(filter);
    filter.connect(gain);
    gain.connect(ctx.destination);
    noise.start(now);
    noise.stop(now + 0.15);
  } else if (evt === "slash") {
    var osc = ctx.createOscillator();
    var gain = ctx.createGain();
    osc.type = "triangle";
    osc.frequency.setValueAtTime(900, now);
    osc.frequency.exponentialRampToValueAtTime(200, now + 0.08);
    gain.gain.setValueAtTime(0.3, now);
    gain.gain.exponentialRampToValueAtTime(0.01, now + 0.08);
    osc.connect(gain);
    gain.connect(ctx.destination);
    osc.start(now);
    osc.stop(now + 0.08);
  } else if (evt === "hit") {
    var osc = ctx.createOscillator();
    var gain = ctx.createGain();
    osc.type = "square";
    osc.frequency.setValueAtTime(180, now);
    osc.frequency.exponentialRampToValueAtTime(50, now + 0.1);
    gain.gain.setValueAtTime(0.25, now);
    gain.gain.exponentialRampToValueAtTime(0.01, now + 0.1);
    osc.connect(gain);
    gain.connect(ctx.destination);
    osc.start(now);
    osc.stop(now + 0.1);
  } else if (evt === "damage") {
    var osc = ctx.createOscillator();
    var gain = ctx.createGain();
    osc.type = "sawtooth";
    osc.frequency.setValueAtTime(140, now);
    osc.frequency.exponentialRampToValueAtTime(35, now + 0.25);
    gain.gain.setValueAtTime(0.35, now);
    gain.gain.exponentialRampToValueAtTime(0.01, now + 0.25);
    osc.connect(gain);
    gain.connect(ctx.destination);
    osc.start(now);
    osc.stop(now + 0.25);
  } else if (evt === "geo") {
    var osc = ctx.createOscillator();
    var gain = ctx.createGain();
    osc.type = "sine";
    osc.frequency.setValueAtTime(1400, now);
    osc.frequency.setValueAtTime(2100, now + 0.04);
    gain.gain.setValueAtTime(0.2, now);
    gain.gain.exponentialRampToValueAtTime(0.01, now + 0.1);
    osc.connect(gain);
    gain.connect(ctx.destination);
    osc.start(now);
    osc.stop(now + 0.1);
  } else if (evt === "focus") {
    var osc = ctx.createOscillator();
    var gain = ctx.createGain();
    osc.type = "sine";
    osc.frequency.setValueAtTime(220, now);
    osc.frequency.exponentialRampToValueAtTime(440, now + 0.3);
    gain.gain.setValueAtTime(0.2, now);
    gain.gain.exponentialRampToValueAtTime(0.01, now + 0.3);
    osc.connect(gain);
    gain.connect(ctx.destination);
    osc.start(now);
    osc.stop(now + 0.3);
  } else if (evt === "menu_move") {
    var osc = ctx.createOscillator();
    var gain = ctx.createGain();
    osc.type = "sine";
    osc.frequency.setValueAtTime(650, now);
    gain.gain.setValueAtTime(0.1, now);
    gain.gain.exponentialRampToValueAtTime(0.01, now + 0.03);
    osc.connect(gain);
    gain.connect(ctx.destination);
    osc.start(now);
    osc.stop(now + 0.03);
  } else if (evt === "menu_select") {
    var osc = ctx.createOscillator();
    var gain = ctx.createGain();
    osc.type = "sine";
    osc.frequency.setValueAtTime(450, now);
    osc.frequency.setValueAtTime(750, now + 0.04);
    gain.gain.setValueAtTime(0.15, now);
    gain.gain.exponentialRampToValueAtTime(0.01, now + 0.08);
    osc.connect(gain);
    gain.connect(ctx.destination);
    osc.start(now);
    osc.stop(now + 0.08);
  }
});

void plat_sound(const char *evt) {
  if (evt) web_play_sound_event(evt);
}

EM_JS(void, web_canvas_present, (const uint16_t *fb), {
  var src = HEAPU16.subarray(fb >> 1, (fb >> 1) + 320 * 240);
  var out = Module.hkImage.data;
  for (var i = 0, j = 0; i < src.length; i++, j += 4) {
    var c = src[i];
    out[j] = ((c >> 11) & 31) * 255 / 31;
    out[j + 1] = ((c >> 5) & 63) * 255 / 63;
    out[j + 2] = (c & 31) * 255 / 31;
    out[j + 3] = 255;
  }
  Module.hkCtx.putImageData(Module.hkImage, 0, 0);
});

EM_JS(void, web_saves_mount, (), {
  if (Module.hkMounting) return;
  try {
    FS.mkdir("/saves");
  } catch (e) {}
  FS.mount(IDBFS, {}, "/saves");
  Module.hkMounting = true;
  FS.syncfs(true, function(err) {
    if (err) console.error(err);
    Module.hkMounting = false;
  });
});

EM_JS(void, web_saves_sync, (), {
  FS.syncfs(false, function(err) {
    if (err) console.error(err);
  });
});

static uint32_t held_keys;

static EM_BOOL key_event(int event_type, const EmscriptenKeyboardEvent *e, void *user) {
  (void)user;
  uint32_t bits = web_get_key_code(e->code);
  if (!bits) return EM_FALSE;
  if (event_type == EMSCRIPTEN_EVENT_KEYDOWN) held_keys |= bits;
  if (event_type == EMSCRIPTEN_EVENT_KEYUP) held_keys &= ~bits;
  return EM_TRUE;
}

uint32_t plat_keys(void) {
  uint32_t r = held_keys;
  if (r) r |= K_ANY;
  return r;
}
uint32_t plat_millis(void) { return (uint32_t)emscripten_get_now(); }
void plat_sleep(uint32_t ms) { (void)ms; }
void plat_push(int x, int y, int w, int h, const uint16_t *px) {
  for (int r = 0; r < h; r++)
    for (int c = 0; c < w; c++)
      if ((unsigned)(x + c) < SCREEN_W && (unsigned)(y + r) < SCREEN_H) web_fb[(y + r) * SCREEN_W + x + c] = px[r * w + c];
}
void plat_fill(int x, int y, int w, int h, uint16_t c) {
  for (int r = 0; r < h; r++)
    for (int k = 0; k < w; k++)
      if ((unsigned)(x + k) < SCREEN_W && (unsigned)(y + r) < SCREEN_H) web_fb[(y + r) * SCREEN_W + x + k] = c;
}
static char save_dir[256] = "/saves";
bool plat_save(const char *name, const void *data, uint32_t len) {
  char p[512];
  snprintf(p, sizeof p, "%s/%s", save_dir, name);
  FILE *f = fopen(p, "wb");
  if (!f) return false;
  fwrite(data, 1, len, f);
  fclose(f);
  web_saves_sync();
  return true;
}
static uint8_t loaded[65536];
const uint8_t *plat_load(const char *name, uint32_t *len) {
  char p[512];
  snprintf(p, sizeof p, "%s/%s", save_dir, name);
  FILE *f = fopen(p, "rb");
  if (!f) return NULL;
  *len = (uint32_t)fread(loaded, 1, sizeof loaded, f);
  fclose(f);
  return loaded;
}
static uint8_t reserved[65536];
uint8_t *plat_reserve(const char *name, uint32_t len) { return len <= sizeof reserved ? reserved : NULL; }
void plat_reserved(const char *name, uint32_t len) {
  (void)name;
  plat_save(name, reserved, len);
}
bool plat_remove(const char *name) {
  char p[512];
  snprintf(p, sizeof p, "%s/%s", save_dir, name);
  bool ok = remove(p) == 0;
  if (ok) web_saves_sync();
  return ok;
}

void plat_begin(void) {
  web_canvas_init();
  web_saves_mount();
  emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, NULL, true, key_event);
  emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, NULL, true, key_event);
}
int plat_end(void) { return 0; }
void web_present(void) { web_canvas_present(web_fb); }
