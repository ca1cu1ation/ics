#include "cpu/exec.h"

make_EHelper(test) {
  rtl_and(&t2, &id_dest->val, &id_src->val);
  rtl_update_ZFSF(&t2, id_dest->width);
  rtl_set_CF(&tzero);
  rtl_set_OF(&tzero);

  print_asm_template2(test);
}

make_EHelper(and) {
  rtl_and(&t2, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t2);
  rtl_update_ZFSF(&t2, id_dest->width);
  rtl_set_CF(&tzero);
  rtl_set_OF(&tzero);

  print_asm_template2(and);
}

make_EHelper(xor) {
  rtl_xor(&t2, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t2);
  rtl_update_ZFSF(&t2, id_dest->width);
  rtl_set_CF(&tzero);
  rtl_set_OF(&tzero);

  print_asm_template2(xor);
}

make_EHelper(or) {
  rtl_or(&t2, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t2);
  rtl_update_ZFSF(&t2, id_dest->width);
  rtl_set_CF(&tzero);
  rtl_set_OF(&tzero);

  print_asm_template2(or);
}

make_EHelper(sar) {
  rtl_sar(&t2, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t2);
  // unnecessary to update CF and OF in NEMU

  print_asm_template2(sar);
}

make_EHelper(shl) {
  rtl_shl(&t2, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t2);
  // unnecessary to update CF and OF in NEMU

  print_asm_template2(shl);
}

make_EHelper(shr) {
  rtl_shr(&t2, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t2);
  // unnecessary to update CF and OF in NEMU

  print_asm_template2(shr);
}

make_EHelper(rol) {
  uint32_t bits = id_dest->width * 8;
  uint32_t cnt = id_src->val & 0x1f;
  if (cnt != 0) {
    cnt %= bits;
    if (cnt != 0) {
      uint32_t mask = (bits == 32) ? 0xffffffffu : ((1u << bits) - 1);
      uint32_t val = id_dest->val & mask;
      t2 = ((val << cnt) | (val >> (bits - cnt))) & mask;
      operand_write(id_dest, &t2);
    }
  }

  print_asm_template2(rol);
}

make_EHelper(setcc) {
  uint8_t subcode = decoding.opcode & 0xf;
  rtl_setcc(&t2, subcode);
  operand_write(id_dest, &t2);

  print_asm("set%s %s", get_cc_name(subcode), id_dest->str);
}

make_EHelper(bsr) {
  uint32_t src = id_src->val;
  int msb = id_src->width * 8 - 1;

  if (id_src->width == 2) {
    src &= 0xffff;
  }

  if (src == 0) {
    rtl_li(&t0, 1);
    rtl_set_ZF(&t0);
  } else {
    while (((src >> msb) & 1) == 0) {
      msb--;
    }
    rtl_li(&t2, msb);
    operand_write(id_dest, &t2);
    rtl_set_ZF(&tzero);
  }

  print_asm_template2(bsr);
}

make_EHelper(not) {
  rtl_xori(&t2, &id_dest->val, 0xffffffff);
  operand_write(id_dest, &t2);

  print_asm_template1(not);
}
