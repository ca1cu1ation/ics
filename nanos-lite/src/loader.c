#include "common.h"

#define DEFAULT_ENTRY ((void *)0x4000000)

void ramdisk_read(void *buf, off_t offset, size_t len);
size_t get_ramdisk_size(void);

uintptr_t loader(_Protect *as, const char *filename) {
  (void)as;
  (void)filename;

  size_t img_size = get_ramdisk_size();
  ramdisk_read(DEFAULT_ENTRY, 0, img_size);

  Log("Program loaded at %p, size = %u", DEFAULT_ENTRY, (uint32_t)img_size);
  return (uintptr_t)DEFAULT_ENTRY;
}
