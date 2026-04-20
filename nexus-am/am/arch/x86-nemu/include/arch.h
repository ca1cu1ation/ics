#ifndef __ARCH_H__
#define __ARCH_H__

#include <am.h>

#define PMEM_SIZE (128 * 1024 * 1024)
#define PGSIZE    4096    // Bytes mapped by a page

struct _RegSet {
  uintptr_t esi, ebx, eax, eip, edx, error_code, eflags, ecx, cs, esp, edi, ebp;
  int       irq;
};

/*
 * Trap frame layout in x86-nemu trap.S (after pushal):
 * [0] edi [1] esi [2] ebp [3] esp [4] ebx [5] edx [6] ecx [7] eax ...
 */
#define SYSCALL_ARG1(r) ((uintptr_t)(((uint32_t *)(r))[7]))
#define SYSCALL_ARG2(r) ((uintptr_t)(((uint32_t *)(r))[4]))
#define SYSCALL_ARG3(r) ((uintptr_t)(((uint32_t *)(r))[6]))
#define SYSCALL_ARG4(r) ((uintptr_t)(((uint32_t *)(r))[5]))

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif
#endif
