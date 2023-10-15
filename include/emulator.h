#ifndef EMULATOR_H
#define EMULATOR_H

#include "builder/dyn_var.h"
#include "builder/static_var.h"

namespace nes {

typedef unsigned short nes_word;
typedef builder::dyn_var<nes_word> dnes_word;

typedef unsigned char nes_byte;
typedef builder::dyn_var<nes_byte> dnes_byte;


typedef builder::dyn_var<unsigned char*> dnes_byte_addr;

struct nes_context_t {
	dnes_byte &acc;
	dnes_byte &xreg;
	dnes_byte &yreg;

	dnes_byte &carry_flag;
	dnes_byte &zero_flag;
	dnes_byte &interrupt_flag;
	dnes_byte &decimal_flag;
	dnes_byte &break_flag;
	dnes_byte &overflow_flag;
	dnes_byte &sign_flag;

	nes_byte* main_memory; // 0xffff
	dnes_byte_addr &dmain_memory;

	nes_context_t (dnes_byte &acc, dnes_byte &xreg, dnes_byte &yreg, 
		dnes_byte &carry_flag, dnes_byte &zero_flag, dnes_byte &interrupt_flag, 
		dnes_byte &decimal_flag, dnes_byte &break_flag, dnes_byte &overflow_flag, 
		dnes_byte &sign_flag, nes_byte* main_memory, dnes_byte_addr &dmain_memory):
		acc(acc), xreg(xreg), yreg(yreg), carry_flag(carry_flag), zero_flag(zero_flag),
		interrupt_flag(interrupt_flag), decimal_flag(decimal_flag), break_flag(break_flag),
		overflow_flag(overflow_flag), sign_flag(sign_flag), main_memory(main_memory), 
		dmain_memory(dmain_memory) {}

};

extern dnes_byte acc;
extern dnes_byte xreg;
extern dnes_byte yreg;
extern dnes_byte carry_flag;
extern dnes_byte zero_flag;
extern dnes_byte interrupt_flag;
extern dnes_byte decimal_flag;
extern dnes_byte break_flag;
extern dnes_byte overflow_flag;
extern dnes_byte sign_flag;

void emulate_code (nes_byte* main_memory, nes_word offset, dnes_byte_addr dmain_memory);


}

#endif
