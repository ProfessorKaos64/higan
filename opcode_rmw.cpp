auto R65816::op_adjust_imm_b(reg16_t& reg, signed adjust) {
L op_io_irq();
  reg.l += adjust;
  regs.p.n = (reg.l & 0x80);
  regs.p.z = (reg.l == 0);
}

auto R65816::op_adjust_imm_w(reg16_t& reg, signed adjust) {
L op_io_irq();
  reg.w += adjust;
  regs.p.n = (reg.w & 0x8000);
  regs.p.z = (reg.w == 0);
}

auto R65816::op_asl_imm_b() {
L op_io_irq();
  regs.p.c = (regs.a.l & 0x80);
  regs.a.l <<= 1;
  regs.p.n = (regs.a.l & 0x80);
  regs.p.z = (regs.a.l == 0);
}

auto R65816::op_asl_imm_w() {
L op_io_irq();
  regs.p.c = (regs.a.w & 0x8000);
  regs.a.w <<= 1;
  regs.p.n = (regs.a.w & 0x8000);
  regs.p.z = (regs.a.w == 0);
}

auto R65816::op_lsr_imm_b() {
L op_io_irq();
  regs.p.c = (regs.a.l & 0x01);
  regs.a.l >>= 1;
  regs.p.n = (regs.a.l & 0x80);
  regs.p.z = (regs.a.l == 0);
}

auto R65816::op_lsr_imm_w() {
L op_io_irq();
  regs.p.c = (regs.a.w & 0x0001);
  regs.a.w >>= 1;
  regs.p.n = (regs.a.w & 0x8000);
  regs.p.z = (regs.a.w == 0);
}

auto R65816::op_rol_imm_b() {
L op_io_irq();
  bool carry = regs.p.c;
  regs.p.c = (regs.a.l & 0x80);
  regs.a.l = (regs.a.l << 1) | carry;
  regs.p.n = (regs.a.l & 0x80);
  regs.p.z = (regs.a.l == 0);
}

auto R65816::op_rol_imm_w() {
L op_io_irq();
  bool carry = regs.p.c;
  regs.p.c = (regs.a.w & 0x8000);
  regs.a.w = (regs.a.w << 1) | carry;
  regs.p.n = (regs.a.w & 0x8000);
  regs.p.z = (regs.a.w == 0);
}

auto R65816::op_ror_imm_b() {
L op_io_irq();
  bool carry = regs.p.c;
  regs.p.c = (regs.a.l & 0x01);
  regs.a.l = (carry << 7) | (regs.a.l >> 1);
  regs.p.n = (regs.a.l & 0x80);
  regs.p.z = (regs.a.l == 0);
}

auto R65816::op_ror_imm_w() {
L op_io_irq();
  bool carry = regs.p.c;
  regs.p.c = (regs.a.w & 0x0001);
  regs.a.w = (carry << 15) | (regs.a.w >> 1);
  regs.p.n = (regs.a.w & 0x8000);
  regs.p.z = (regs.a.w == 0);
}

auto R65816::op_adjust_addr_b(fp op) {
  aa.l = op_readpc();
  aa.h = op_readpc();
  rd.l = op_readdbr(aa.w);
  op_io();
  call(op);
L op_writedbr(aa.w, rd.l);
}

auto R65816::op_adjust_addr_w(fp op) {
  aa.l = op_readpc();
  aa.h = op_readpc();
  rd.l = op_readdbr(aa.w + 0);
  rd.h = op_readdbr(aa.w + 1);
  op_io();
  call(op);
  op_writedbr(aa.w + 1, rd.h);
L op_writedbr(aa.w + 0, rd.l);
}

auto R65816::op_adjust_addrx_b(fp op) {
  aa.l = op_readpc();
  aa.h = op_readpc();
  op_io();
  rd.l = op_readdbr(aa.w + regs.x.w);
  op_io();
  call(op);
L op_writedbr(aa.w + regs.x.w, rd.l);
}

auto R65816::op_adjust_addrx_w(fp op) {
  aa.l = op_readpc();
  aa.h = op_readpc();
  op_io();
  rd.l = op_readdbr(aa.w + regs.x.w + 0);
  rd.h = op_readdbr(aa.w + regs.x.w + 1);
  op_io();
  call(op);
  op_writedbr(aa.w + regs.x.w + 1, rd.h);
L op_writedbr(aa.w + regs.x.w + 0, rd.l);
}

auto R65816::op_adjust_dp_b(fp op) {
  dp = op_readpc();
  op_io_cond2();
  rd.l = op_readdp(dp);
  op_io();
  call(op);
L op_writedp(dp, rd.l);
}

auto R65816::op_adjust_dp_w(fp op) {
  dp = op_readpc();
  op_io_cond2();
  rd.l = op_readdp(dp + 0);
  rd.h = op_readdp(dp + 1);
  op_io();
  call(op);
  op_writedp(dp + 1, rd.h);
L op_writedp(dp + 0, rd.l);
}

auto R65816::op_adjust_dpx_b(fp op) {
  dp = op_readpc();
  op_io_cond2();
  op_io();
  rd.l = op_readdp(dp + regs.x.w);
  op_io();
  call(op);
L op_writedp(dp + regs.x.w, rd.l);
}

auto R65816::op_adjust_dpx_w(fp op) {
  dp = op_readpc();
  op_io_cond2();
  op_io();
  rd.l = op_readdp(dp + regs.x.w + 0);
  rd.h = op_readdp(dp + regs.x.w + 1);
  op_io();
  call(op);
  op_writedp(dp + regs.x.w + 1, rd.h);
L op_writedp(dp + regs.x.w + 0, rd.l);
}
