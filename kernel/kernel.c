#include "pic.h"
#include <am.h>
#include <amdev.h>
#include <klib-macros.h>
#include <klib.h>

#define SIDE 16

static int w, h; // Screen size

#define KEYNAME(key) [AM_KEY_##key] = #key,
static const char *key_names[] = {AM_KEYS(KEYNAME)};

static inline void puts(const char *s) {
    for (; *s; s++)
        putch(*s);
}

void print_key() {
    AM_INPUT_KEYBRD_T event = {.keycode = AM_KEY_NONE};
    ioe_read(AM_INPUT_KEYBRD, &event);
    if (event.keycode != AM_KEY_NONE && event.keydown) {
        if (event.keycode == AM_KEY_ESCAPE) {
            halt(0);
        }
        puts("Key pressed: ");
        puts(key_names[event.keycode]);
        puts("\n");
    }
}

/*static void draw_tile(int x, int y, int w, int h, uint32_t color) {
  uint32_t pixels[w * h]; // WARNING: large stack-allocated memory
  AM_GPU_FBDRAW_T event = {
    .x = x, .y = y, .w = w, .h = h, .sync = 1,
    .pixels = pixels,
  };
  for (int i = 0; i < w * h; i++) {
    pixels[i] = color;
  }
  ioe_write(AM_GPU_FBDRAW, &event);
}

void splash() {
  AM_GPU_CONFIG_T info = {0};
  ioe_read(AM_GPU_CONFIG, &info);
  w = info.width;
  h = info.height;

  for (int x = 0; x * SIDE <= w; x ++) {
    for (int y = 0; y * SIDE <= h; y++) {
      if ((x & 1) ^ (y & 1)) {
        draw_tile(x * SIDE, y * SIDE, SIDE, SIDE, 0xffffff); // white
      }
    }
  }
}*/

void splash() {
    AM_GPU_CONFIG_T info = {0};
    ioe_read(AM_GPU_CONFIG, &info);
    w = info.width;
    h = info.height;

    for (int x = 0; x < w; x++) {
        for (int y = 0; y < h; y++) {
            int pic_x = pic_width * x / w;
            int pic_y = pic_height * y / h;
            uint32_t pixels[1];
            uint8_t *pic_src =
                pic_bin + (pic_x + pic_y * pic_width) * sizeof(uint32_t);
            memcpy(pixels, pic_src, 4);
            AM_GPU_FBDRAW_T event = {
                .x = x, .y = y, .w = 1, .h = 1, .sync = 0, .pixels = pixels};
            ioe_write(AM_GPU_FBDRAW, &event);
        }
    }

    io_write(AM_GPU_FBDRAW, 0, 0, NULL, 0, 0, true);
}

// Operating system is a C program!
int main(const char *args) {
    ioe_init();

    puts("mainargs = \"");
    puts(args); // make run mainargs=xxx
    puts("\"\n");

    splash();

    puts("Press any key to see its key code...\n");
    while (1) {
        print_key();
    }
    return 0;
}
