#include "fs.h"

void ramdisk_read(void *buf, off_t offset, size_t len);

typedef struct {
  char *name;
  size_t size;
  off_t disk_offset;
  off_t open_offset;
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB, FD_EVENTS, FD_DISPINFO, FD_NORMAL};

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  {"stdin (note that this is not the actual stdin)", 0, 0, 0},
  {"stdout (note that this is not the actual stdout)", 0, 0, 0},
  {"stderr (note that this is not the actual stderr)", 0, 0, 0},
  [FD_FB] = {"/dev/fb", 0, 0, 0},
  [FD_EVENTS] = {"/dev/events", 0, 0, 0},
  [FD_DISPINFO] = {"/proc/dispinfo", 128, 0, 0},
#include "files.h"
};

#define NR_FILES (sizeof(file_table) / sizeof(file_table[0]))

int fs_open(const char *pathname, int flags, int mode) {
  (void)flags;
  (void)mode;

  for (int fd = 0; fd < NR_FILES; fd++) {
    if (strcmp(file_table[fd].name, pathname) == 0) {
      file_table[fd].open_offset = 0;
      return fd;
    }
  }
  return -1;
}

size_t fs_read(int fd, void *buf, size_t len) {
  assert(0 <= fd && fd < NR_FILES);

  Finfo *f = &file_table[fd];
  if (f->open_offset >= f->size) {
    return 0;
  }

  size_t remain = f->size - f->open_offset;
  size_t nread = len < remain ? len : remain;
  ramdisk_read(buf, f->disk_offset + f->open_offset, nread);
  f->open_offset += nread;
  return nread;
}

int fs_close(int fd) {
  assert(0 <= fd && fd < NR_FILES);
  file_table[fd].open_offset = 0;
  return 0;
}

void init_fs() {
  // TODO: initialize the size of /dev/fb
}
