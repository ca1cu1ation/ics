#include "common.h"
#include "fs.h"

#define DEFAULT_ENTRY ((void *)0x4000000)

uintptr_t loader(_Protect *as, const char *filename) {
  (void)as;

  int fd = fs_open(filename, 0, 0);
  assert(fd >= 0);

  size_t img_size = 0;
  while (1) {
    size_t nread = fs_read(fd, (uint8_t *)DEFAULT_ENTRY + img_size, 4096);
    if (nread == 0) {
      break;
    }
    img_size += nread;
  }
  fs_close(fd);

  Log("Program loaded at %p, size = %u", DEFAULT_ENTRY, (uint32_t)img_size);
  return (uintptr_t)DEFAULT_ENTRY;
}
