#include "cpu/exec.h"
#include "memory/mmu.h"

void raise_intr(uint8_t NO, vaddr_t ret_addr) {
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * That is, use ``NO'' to index the IDT.
   */
  uint32_t gate_addr = cpu.idtr.base + NO * 8;
  assert((NO * 8 + 7) <= cpu.idtr.limit);

  uint32_t low = vaddr_read(gate_addr, 4);
  uint32_t high = vaddr_read(gate_addr + 4, 4);
  vaddr_t handler = (high & 0xffff0000) | (low & 0x0000ffff);

  /* iret in this framework pops eip first, then eflags. */
  rtlreg_t temp = cpu.eflags;
  rtl_push(&temp);
  temp = ret_addr;
  rtl_push(&temp);

  cpu.eip = handler;
}

void dev_raise_intr() {
}
