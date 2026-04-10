#include "cpu/exec.h"

static inline void push_by_width(const rtlreg_t *src, int width) {
  if (width == 2) {
    reg_w(R_SP) -= 2;
    t0 = *src & 0xffff;
    vaddr_write(reg_w(R_SP), 2, t0);
  }
  else {
    rtl_push(src);
  }
}

static inline void pop_by_width(rtlreg_t *dest, int width) {
  if (width == 2) {
    *dest = vaddr_read(reg_w(R_SP), 2);
    reg_w(R_SP) += 2;
  }
  else {
    rtl_pop(dest);
  }
}

make_EHelper(mov) {
  operand_write(id_dest, &id_src->val);
  print_asm_template2(mov);
}

make_EHelper(movs) {
  int width = decoding.dest.width;
  t0 = vaddr_read(reg_l(R_ESI), width);
  vaddr_write(reg_l(R_EDI), width, t0);

  reg_l(R_ESI) += width;
  reg_l(R_EDI) += width;

  print_asm("movs%c", suffix_char(width));
}

make_EHelper(xchg) {
  if (decoding.opcode >= 0x90 && decoding.opcode <= 0x97) {
    rtl_lr(&t0, R_EAX, id_dest->width);
    operand_write(id_dest, &t0);
    rtl_sr(R_EAX, id_dest->width, &id_dest->val);

    print_asm("xchg%c %%%s,%s", suffix_char(id_dest->width), reg_name(R_EAX, id_dest->width), id_dest->str);
    return;
  }

  t0 = id_dest->val;
  operand_write(id_dest, &id_src->val);
  operand_write(id_src, &t0);

  print_asm_template2(xchg);
}

make_EHelper(push) {
  push_by_width(&id_dest->val, id_dest->width);

  print_asm_template1(push);
}

make_EHelper(pop) {
  pop_by_width(&t0, id_dest->width);
  operand_write(id_dest, &t0);

  print_asm_template1(pop);
}

make_EHelper(pusha) {
  if (decoding.is_operand_size_16) {
    rtlreg_t temp_sp = reg_w(R_SP);
    t0 = reg_w(R_AX); push_by_width(&t0, 2);
    t0 = reg_w(R_CX); push_by_width(&t0, 2);
    t0 = reg_w(R_DX); push_by_width(&t0, 2);
    t0 = reg_w(R_BX); push_by_width(&t0, 2);
    t0 = temp_sp;     push_by_width(&t0, 2);
    t0 = reg_w(R_BP); push_by_width(&t0, 2);
    t0 = reg_w(R_SI); push_by_width(&t0, 2);
    t0 = reg_w(R_DI); push_by_width(&t0, 2);
  }
  else {
    rtlreg_t temp_esp = reg_l(R_ESP);
    t0 = reg_l(R_EAX); push_by_width(&t0, 4);
    t0 = reg_l(R_ECX); push_by_width(&t0, 4);
    t0 = reg_l(R_EDX); push_by_width(&t0, 4);
    t0 = reg_l(R_EBX); push_by_width(&t0, 4);
    t0 = temp_esp;     push_by_width(&t0, 4);
    t0 = reg_l(R_EBP); push_by_width(&t0, 4);
    t0 = reg_l(R_ESI); push_by_width(&t0, 4);
    t0 = reg_l(R_EDI); push_by_width(&t0, 4);
  }

  print_asm("pusha");
}

make_EHelper(popa) {
  if (decoding.is_operand_size_16) {
    pop_by_width(&t0, 2); reg_w(R_DI) = t0;
    pop_by_width(&t0, 2); reg_w(R_SI) = t0;
    pop_by_width(&t0, 2); reg_w(R_BP) = t0;
    pop_by_width(&t0, 2); /* skip SP */
    pop_by_width(&t0, 2); reg_w(R_BX) = t0;
    pop_by_width(&t0, 2); reg_w(R_DX) = t0;
    pop_by_width(&t0, 2); reg_w(R_CX) = t0;
    pop_by_width(&t0, 2); reg_w(R_AX) = t0;
  }
  else {
    pop_by_width(&t0, 4); reg_l(R_EDI) = t0;
    pop_by_width(&t0, 4); reg_l(R_ESI) = t0;
    pop_by_width(&t0, 4); reg_l(R_EBP) = t0;
    pop_by_width(&t0, 4); /* skip ESP */
    pop_by_width(&t0, 4); reg_l(R_EBX) = t0;
    pop_by_width(&t0, 4); reg_l(R_EDX) = t0;
    pop_by_width(&t0, 4); reg_l(R_ECX) = t0;
    pop_by_width(&t0, 4); reg_l(R_EAX) = t0;
  }

  print_asm("popa");
}

make_EHelper(leave) {
  if (decoding.is_operand_size_16) {
    reg_w(R_SP) = reg_w(R_BP);
    pop_by_width(&t0, 2);
    reg_w(R_BP) = t0;
  }
  else {
    reg_l(R_ESP) = reg_l(R_EBP);
    pop_by_width(&t0, 4);
    reg_l(R_EBP) = t0;
  }

  print_asm("leave");
}

make_EHelper(cltd) {
  if (decoding.is_operand_size_16) {
    reg_w(R_DX) = (reg_w(R_AX) & 0x8000) ? 0xffff : 0x0000;
  }
  else {
    reg_l(R_EDX) = (int32_t)reg_l(R_EAX) < 0 ? 0xffffffff : 0x00000000;
  }

  print_asm(decoding.is_operand_size_16 ? "cwtl" : "cltd");
}

make_EHelper(cwtl) {
  if (decoding.is_operand_size_16) {
    reg_w(R_AX) = (int8_t)reg_b(R_AL);
  }
  else {
    reg_l(R_EAX) = (int16_t)reg_w(R_AX);
  }

  print_asm(decoding.is_operand_size_16 ? "cbtw" : "cwtl");
}

make_EHelper(movsx) {
  id_dest->width = decoding.is_operand_size_16 ? 2 : 4;
  rtl_sext(&t2, &id_src->val, id_src->width);
  operand_write(id_dest, &t2);
  print_asm_template2(movsx);
}

make_EHelper(movzx) {
  id_dest->width = decoding.is_operand_size_16 ? 2 : 4;
  operand_write(id_dest, &id_src->val);
  print_asm_template2(movzx);
}

make_EHelper(lea) {
  rtl_li(&t2, id_src->addr);
  operand_write(id_dest, &t2);
  print_asm_template2(lea);
}
