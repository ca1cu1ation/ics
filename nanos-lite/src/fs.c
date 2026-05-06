#include "fs.h"

void ramdisk_read(void *buf, off_t offset, size_t len);
void ramdisk_write(const void *buf, off_t offset, size_t len);
size_t events_read(void *buf, size_t len);
void dispinfo_read(void *buf, off_t offset, size_t len);
void fb_write(const void *buf, off_t offset, size_t len);

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

  if (fd == FD_EVENTS) {
    size_t nread = 0;
    while (nread == 0) {
      nread = events_read(buf, len);
    }
    return nread;
  }

  if (fd == FD_DISPINFO) {
    Finfo *f = &file_table[fd];
    if (f->open_offset >= f->size) {
      return 0;
    }
    size_t remain = f->size - f->open_offset;
    size_t nread = len < remain ? len : remain;
    dispinfo_read(buf, f->open_offset, nread);
    f->open_offset += nread;
    return nread;
  }

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

size_t fs_write(int fd, const void *buf, size_t len) {
  assert(0 <= fd && fd < NR_FILES);

  if (fd == FD_EVENTS) {
    return 0;
  }

  if (fd == FD_STDOUT || fd == FD_STDERR) {
    const char *cbuf = (const char *)buf;
    for (size_t i = 0; i < len; i++) {
      _putc(cbuf[i]);
    }
    return len;
  }

  if (fd == FD_FB) {
    Finfo *f = &file_table[fd];
    fb_write(buf, f->open_offset, len);
    f->open_offset += len;
    return len;
  }

  Finfo *f = &file_table[fd];
  if (f->open_offset >= f->size) {
    return 0;
  }

  size_t remain = f->size - f->open_offset;
  size_t nwrite = len < remain ? len : remain;
  ramdisk_write(buf, f->disk_offset + f->open_offset, nwrite);
  f->open_offset += nwrite;
  return nwrite;
}

off_t fs_lseek(int fd, off_t offset, int whence) {
  assert(0 <= fd && fd < NR_FILES);

  Finfo *f = &file_table[fd];
  off_t new_offset;

  switch (whence) {
    case SEEK_SET: new_offset = offset; break;
    case SEEK_CUR: new_offset = f->open_offset + offset; break;
    case SEEK_END: new_offset = f->size + offset; break;
    default: return -1;
  }

  if (new_offset < 0) {
    return -1;
  }

  if (new_offset > (off_t)f->size) {
    new_offset = f->size;
  }

  f->open_offset = new_offset;
  return f->open_offset;
}

int fs_close(int fd) {
  assert(0 <= fd && fd < NR_FILES);
  file_table[fd].open_offset = 0;
  return 0;
}

void init_fs() {
  file_table[FD_FB].size = _screen.width * _screen.height * sizeof(uint32_t);
}
