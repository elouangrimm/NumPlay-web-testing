/* Web platform: browser keyboard, framebuffer output and save files. */
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
  c.style.imageRendering = "pixelated";
  Module.hkCtx = c.getContext("2d", {alpha: false});
  Module.hkImage = Module.hkCtx.createImageData(320, 240);
});

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

static uint32_t key_code(const char *code) {
  if (!code) return 0;
  if (!strcmp(code, "ArrowLeft")) return K_LEFT;
  if (!strcmp(code, "ArrowRight")) return K_RIGHT;
  if (!strcmp(code, "ArrowUp")) return K_UP;
  if (!strcmp(code, "ArrowDown")) return K_DOWN;
  if (!strcmp(code, "Space")) return K_JUMP | K_OK;
  if (!strcmp(code, "Enter")) return K_OK;
  if (!strcmp(code, "KeyX")) return K_ATTACK | K_BACK;
  if (!strcmp(code, "ShiftLeft") || !strcmp(code, "ShiftRight")) return K_DASH;
  if (!strcmp(code, "KeyC") || !strcmp(code, "KeyZ")) return K_SPELL | K_FOCUS;
  if (!strcmp(code, "Backspace")) return K_PAUSE;
  if (!strcmp(code, "KeyI")) return K_INV;
  if (!strcmp(code, "Tab")) return K_MAP;
  if (!strcmp(code, "Escape")) return K_HOME;
  return 0;
}

static EM_BOOL key_event(int event_type, const EmscriptenKeyboardEvent *e, void *user) {
  (void)user;
  uint32_t bits = key_code(e->code);
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
