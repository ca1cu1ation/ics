#include <am.h>
#include <x86.h>

#define RTC_PORT 0x48   // Note that this is not standard
#define SYNC_PORT 0x100
#define I8042_DATA_PORT 0x60
#define I8042_STATUS_PORT 0x64
#define I8042_STATUS_HASKEY_MASK 0x1
static unsigned long boot_time;

void _ioe_init() {
  boot_time = inl(RTC_PORT);
}

unsigned long _uptime() {
  unsigned long now = inl(RTC_PORT);
  return now - boot_time;
}

uint32_t* const fb = (uint32_t *)0x40000;

_Screen _screen = {
  .width  = 400,
  .height = 300,
};

extern void* memcpy(void *, const void *, int);

static inline int min(int a, int b) {
  return a < b ? a : b;
}

void _draw_rect(const uint32_t *pixels, int x, int y, int w, int h) {
  if (x >= _screen.width || y >= _screen.height || w <= 0 || h <= 0) {
    return;
  }

  int cp_bytes = sizeof(uint32_t) * min(w, _screen.width - x);
  for (int j = 0; j < h && y + j < _screen.height; j++) {
    memcpy(&fb[(y + j) * _screen.width + x], pixels, cp_bytes);
    pixels += w;
  }
}

void _draw_sync() {
  outl(SYNC_PORT, 1);
}

int _read_key() {
  if ((inb(I8042_STATUS_PORT) & I8042_STATUS_HASKEY_MASK) == 0) {
    return _KEY_NONE;
  }
  return inl(I8042_DATA_PORT);
}
