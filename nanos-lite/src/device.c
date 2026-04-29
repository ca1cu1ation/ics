#include "common.h"

#define NAME(key) \
  [_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [_KEY_NONE] = "NONE",
  _KEYS(NAME)
};

size_t events_read(void *buf, size_t len) {
  static unsigned long last_uptime = 0;
  char tmp[64];

  int keycode = _read_key();
  if (keycode != _KEY_NONE) {
    int is_down = (keycode & 0x8000) != 0;
    int code = keycode & ~0x8000;
    int n = snprintf(tmp, sizeof(tmp), "%s %s\n", is_down ? "kd" : "ku", keyname[code]);
    if (n <= 0) {
      return 0;
    }
    size_t nwrite = (size_t)n < len ? (size_t)n : len;
    memcpy(buf, tmp, nwrite);
    return nwrite;
  }

  unsigned long now = _uptime();
  if (now - last_uptime >= 1000 / 30) {
    last_uptime = now;
    int n = snprintf(tmp, sizeof(tmp), "t %lu\n", now);
    if (n <= 0) {
      return 0;
    }
    size_t nwrite = (size_t)n < len ? (size_t)n : len;
    memcpy(buf, tmp, nwrite);
    return nwrite;
  }

  return 0;
}

static char dispinfo[128] __attribute__((used));

void dispinfo_read(void *buf, off_t offset, size_t len) {
  memcpy(buf, dispinfo + offset, len);
}

void fb_write(const void *buf, off_t offset, size_t len) {
  const uint32_t *pixels = (const uint32_t *)buf;
  size_t pixel_offset = offset / sizeof(uint32_t);
  size_t pixel_count = len / sizeof(uint32_t);

  if (pixel_count == 0) {
    return;
  }

  int x = pixel_offset % _screen.width;
  int y = pixel_offset / _screen.width;

  while (pixel_count > 0 && y < _screen.height) {
    int row_space = _screen.width - x;
    int row_pixels = (int)(pixel_count < (size_t)row_space ? pixel_count : (size_t)row_space);
    _draw_rect(pixels, x, y, row_pixels, 1);
    pixels += row_pixels;
    pixel_count -= row_pixels;
    x = 0;
    y++;
  }
}

void init_device() {
  _ioe_init();

  // TODO: print the string to array `dispinfo` with the format
  // described in the Navy-apps convention
  snprintf(dispinfo, sizeof(dispinfo), "WIDTH:%d\nHEIGHT:%d\n", _screen.width, _screen.height);
}
