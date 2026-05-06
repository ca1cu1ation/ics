#include "common.h"

#define NAME(key) \
  [_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [_KEY_NONE] = "NONE",
  _KEYS(NAME)
};

size_t events_read(void *buf, size_t len) {
  static unsigned long last_uptime = 0;
  static char pending[64];
  static size_t pending_len = 0;
  static size_t pending_pos = 0;

  if (len == 0) {
    return 0;
  }

  if (pending_pos >= pending_len) {
    while (pending_pos >= pending_len) {
      int keycode = _read_key();
      if (keycode != _KEY_NONE) {
        int is_down = (keycode & 0x8000) != 0;
        int code = keycode & ~0x8000;
        const char *name = "NONE";
        if (code >= 0 && code < (int)(sizeof(keyname) / sizeof(keyname[0])) && keyname[code]) {
          name = keyname[code];
        }
        int n = snprintf(pending, sizeof(pending), "%s %s\n", is_down ? "kd" : "ku", name);
        if (n <= 0) {
          pending_len = pending_pos = 0;
          continue;
        }
        pending_len = (size_t)n;
        if (pending_len >= sizeof(pending)) {
          pending_len = sizeof(pending) - 1;
          pending[pending_len - 1] = '\n';
          pending[pending_len] = '\0';
        }
        pending_pos = 0;
        break;
      }

      unsigned long now = _uptime();
      if (now - last_uptime >= 1000 / 30) {
        last_uptime = now;
        int n = snprintf(pending, sizeof(pending), "t %u\n", (unsigned int)now);
        if (n <= 0 || n >= (int)sizeof(pending)) {
          pending_len = pending_pos = 0;
          continue;
        }
        pending_len = (size_t)n;
        if (pending_len >= sizeof(pending)) {
          pending_len = sizeof(pending) - 1;
          pending[pending_len - 1] = '\n';
          pending[pending_len] = '\0';
        }
        pending_pos = 0;
        break;
      }
    }
  }

  size_t remain = pending_len - pending_pos;
  size_t nwrite = remain < len ? remain : len;
  memcpy(buf, pending + pending_pos, nwrite);
  pending_pos += nwrite;
  return nwrite;
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
