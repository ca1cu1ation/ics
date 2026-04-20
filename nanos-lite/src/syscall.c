#include "common.h"
#include "syscall.h"

_RegSet* do_syscall(_RegSet *r) {
  uintptr_t a[4];
  a[0] = SYSCALL_ARG1(r);
  a[1] = SYSCALL_ARG2(r);
  a[2] = SYSCALL_ARG3(r);
  a[3] = SYSCALL_ARG4(r);

  switch (a[0]) {
    case SYS_none:
      r->eax = 1;
      break;
    case SYS_exit:
      _halt((int)a[1]);
      break;
    case SYS_write:
      if (a[1] == 1 || a[1] == 2) {
        const char *buf = (const char *)a[2];
        size_t len = (size_t)a[3];
        size_t i;
        for (i = 0; i < len; i++) {
          _putc(buf[i]);
        }
        r->eax = len;
      } else {
        r->eax = (uintptr_t)-1;
      }
      break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }

  return r;
}
