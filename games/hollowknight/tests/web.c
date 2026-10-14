/* Browser runner for the HOST game build. */
#include <emscripten.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "../src/game.h"

void plat_begin(void);
int plat_end(void);
void web_present(void);

static uint8_t *hk_data;

static bool load_data(void) {
  FILE *f = fopen("/data.bin", "rb");
  if (!f) return false;
  if (fseek(f, 0, SEEK_END)) {
    fclose(f);
    return false;
  }
  long n = ftell(f);
  if (n <= 0) {
    fclose(f);
    return false;
  }
  rewind(f);
  hk_data = (uint8_t *)malloc((size_t)n);
  if (!hk_data) {
    fclose(f);
    return false;
  }
  bool ok = fread(hk_data, 1, (size_t)n, f) == (size_t)n;
  fclose(f);
  if (!ok) {
    free(hk_data);
    hk_data = NULL;
  }
  return ok;
}

#define TICK_MS 20
#define MAX_TICKS 4
#define SLOW_MS 34
#define QUICK_MS 18

static bool running;
static uint32_t last, acc, avg8;

static void frame(void) {
  uint32_t keys = plat_keys();
  if (keys & K_HOME) {
    if (menu_in_game() && !g_pd.disable_pause && !game_changing_room()) save_game();
    running = false;
  }
  if (menu_quit()) running = false;
  if (!running) {
    emscripten_cancel_main_loop();
    return;
  }
  uint32_t now = plat_millis();
  acc += now - last, last = now;
  int ticks = 0;
  while (acc >= TICK_MS && ticks < MAX_TICKS) {
    if (!menu_tick(keys)) game_tick(stag_tick(shop_tick(inv_tick(keys))));
    acc -= TICK_MS, ticks++;
  }
  if (ticks == MAX_TICKS) acc = 0;
  if (!ticks) return;
  g_gfx_no_room = !menu_in_game();
  if (menu_in_game()) game_draw_layers();
  menu_draw();
  gfx_frame();
  web_present();
  // On web/PC, disable half-height rendering skips for maximum graphic fidelity
  g_gfx_fast = false;
}

int main(void) {
  uint16_t scratch[SCRATCH_N] __attribute__((aligned(4)));
  g_scratch = scratch;
  if (!load_data()) {
    fprintf(stderr, "Failed to load /data.bin (build with make data and make web)\n");
    return 1;
  }
  hk_bin = hk_data;
  plat_begin();
  plat_fill(0, 0, SCREEN_W, SCREEN_H, 0);
  menu_start();
  last = plat_millis();
  running = true;
  emscripten_set_main_loop(frame, 0, true);
  return plat_end();
}
