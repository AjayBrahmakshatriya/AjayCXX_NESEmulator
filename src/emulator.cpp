#include "emulator.h"
#include "instructions.h"
#include <iostream>
namespace nes {

dnes_byte acc = builder::as_global("reg_acc");
dnes_byte xreg = builder::as_global("reg_x");
dnes_byte yreg = builder::as_global("reg_y");
dnes_byte carry_flag = builder::as_global("reg_flag_c");
dnes_byte zero_flag = builder::as_global("reg_flag_z");
dnes_byte interrupt_flag = builder::as_global("reg_flag_i");
dnes_byte decimal_flag = builder::as_global("reg_flag_d");
dnes_byte break_flag = builder::as_global("reg_flag_b");
dnes_byte overflow_flag = builder::as_global("reg_flag_v");
dnes_byte sign_flag = builder::as_global("reg_flag_s");

static dnes_byte load_byte (nes_context_t context, nes_byte* instruction_addr, instruction::nes_instruction_t instruction) {
	instruction_addr += 1;
	nes_word addr;
	switch (instruction.addressing_mode) {
		case instruction::AM_hash:
			return instruction_addr[0];		
		case instruction::AM_abs:
			addr = (instruction_addr[1]) << 8 | instruction_addr[0];
			return context.dmain_memory[addr];	
		case instruction::AM_zpg:
			addr = instruction_addr[0];
			return context.dmain_memory[addr];
		case instruction::AM_abs_X:
			addr = (instruction_addr[1]) << 8 | instruction_addr[0];
			return context.dmain_memory[context.xreg + addr];	
		case instruction::AM_abs_Y:
			addr = (instruction_addr[1]) << 8 | instruction_addr[0];
			return context.dmain_memory[context.yreg + addr];	
		case instruction::AM_zpg_X:
			addr = instruction_addr[0];
			return context.dmain_memory[context.xreg + addr];
		case instruction::AM_X_ind:
			addr = instruction_addr[0];
			{ 
				dnes_word daddr_high = context.dmain_memory[((context.xreg + addr + 1) % 0xff)];
				dnes_word daddr_low = context.dmain_memory[((context.xreg + addr) % 0xff)];
				dnes_word daddr = (daddr_high << 8) | daddr_low;	
			
				return context.dmain_memory[daddr];
			}	
		case instruction::AM_ind_Y:
			addr = instruction_addr[0];
			{ 
				dnes_word daddr_high = context.dmain_memory[((addr + 1) % 0xff)];
				dnes_word daddr_low = context.dmain_memory[((addr) % 0xff)];
				dnes_word daddr = (daddr_high << 8) | daddr_low;	
			
				return context.dmain_memory[context.yreg + daddr];
			}	
		default: 
			std::cerr << "Invalid addressing mode " << instruction.addressing_mode << std::endl;
			assert(false);	
	}
	return 0;
}

static void store_byte (nes_context_t context, nes_byte* instruction_addr, instruction::nes_instruction_t instruction, 
		dnes_byte data) {
	instruction_addr += 1;
	nes_word addr;
	switch (instruction.addressing_mode) {
		case instruction::AM_abs:
			addr = (instruction_addr[1]) << 8 | instruction_addr[0];
			context.dmain_memory[addr] = data;
			break;
		case instruction::AM_zpg:
			addr = instruction_addr[0];
			context.dmain_memory[addr] = data;
			break;
		case instruction::AM_abs_X:
			addr = (instruction_addr[1]) << 8 | instruction_addr[0];
			context.dmain_memory[context.xreg + addr] = data;
			break;
		case instruction::AM_abs_Y:
			addr = (instruction_addr[1]) << 8 | instruction_addr[0];
			context.dmain_memory[context.yreg + addr] = data;
			break;
		case instruction::AM_zpg_X:
			addr = instruction_addr[0];
			context.dmain_memory[context.xreg + addr] = data;
			break;
		case instruction::AM_X_ind:
			addr = instruction_addr[0];
			{ 
				dnes_word daddr_high = context.dmain_memory[((context.xreg + addr + 1) % 0xff)];
				dnes_word daddr_low = context.dmain_memory[((context.xreg + addr) % 0xff)];
				dnes_word daddr = (daddr_high << 8) | daddr_low;	
			
				context.dmain_memory[daddr] = data;
			}	
			break;
		case instruction::AM_ind_Y:
			addr = instruction_addr[0];
			{ 
				dnes_word daddr_high = context.dmain_memory[((addr + 1) % 0xff)];
				dnes_word daddr_low = context.dmain_memory[((addr) % 0xff)];
				dnes_word daddr = (daddr_high << 8) | daddr_low;	
			
				context.dmain_memory[context.yreg + daddr] = data;
			}	
			break;
		default: 
			std::cerr << "Invalid addressing mode " << instruction.addressing_mode << std::endl;
			assert(false);	
	}
}

static void set_N (nes_context_t context, dnes_byte &output) {
	if (output < 0) context.sign_flag = 1;
	else context.sign_flag = 0;
}
static void set_Z (nes_context_t context, dnes_byte &output) {
	if (output == 0) context.zero_flag = 1;
	else context.zero_flag = 0;
}

static int get_relative_offset(nes_byte d) {
	if (d & 0x80) return -(d & 0x7f);
	else return (d & 0x7f);
}

void emulate_code (nes_byte* main_memory, nes_word offset, dnes_byte_addr dmain_memory) {

	dnes_byte lacc = acc;
	dnes_byte lxreg = xreg;
	dnes_byte lyreg = yreg;
	dnes_byte lcarry_flag = carry_flag;
	dnes_byte lzero_flag = zero_flag;
	dnes_byte linterrupt_flag = interrupt_flag;
	dnes_byte ldecimal_flag = decimal_flag;
	dnes_byte lbreak_flag = break_flag;
	dnes_byte loverflow_flag = overflow_flag;
	dnes_byte lsign_flag = sign_flag;

	struct nes_context_t context (lacc, lxreg, lyreg, lcarry_flag, lzero_flag, linterrupt_flag, 
		ldecimal_flag, lbreak_flag, loverflow_flag, lsign_flag, main_memory, dmain_memory);	

	builder::static_var<nes_word> PC = offset;	

	while (1) {
		nes_byte opcode = main_memory[PC];
		instruction::nes_instruction_t instruction = instruction::nes_instructions[opcode];

		std::string name;
		if (instruction.name == NULL)
			name = "XXX";			
		else
			name = instruction.name;
	
		if (name == "LDA") {
			dnes_byte data = load_byte(context, &main_memory[PC], instruction);
			context.acc = data;
			set_N(context, data); 
			set_Z(context, data); 
		} else if (name == "STA") {
			store_byte(context, &main_memory[PC], instruction, context.acc);
		} else if (name == "BNE") {
			if (!context.zero_flag)  {
				nes_byte off = main_memory[PC+1];
				PC += get_relative_offset(main_memory[PC+1]);
				continue;
			}
		} else if (name == "ADC") {
			dnes_byte data = load_byte(context, &main_memory[PC], instruction);
			dnes_word res = (dnes_word) (context.acc) + data + context.carry_flag;
			context.acc = res | 0xff;
			context.carry_flag = !!(res >> 8);
			if (context.carry_flag) context.overflow_flag = 1;
			set_N(context, context.acc); 
			set_Z(context, context.acc); 
		} else if (name == "AND") {
			dnes_byte data = load_byte(context, &main_memory[PC], instruction);
			context.acc = context.acc & data;
			set_N(context, context.acc); 
			set_Z(context, context.acc); 
		} else if (name == "ASL") {
			dnes_word res = context.acc;	
			res = res << 1;
			context.acc = res | 0xff;
			context.carry_flag = !!(res >> 8);
			set_N(context, context.acc); 
			set_Z(context, context.acc); 
		} else if (name == "DEX") {
			context.xreg = context.xreg - 1;
			set_N(context, context.xreg);
			set_Z(context, context.xreg);
		} else if (name == "BCC") {
			if (!context.carry_flag)  {
				nes_byte off = main_memory[PC+1];
				PC += get_relative_offset(main_memory[PC+1]);
				continue;
			}
		} else if (name == "EXT") {
			return;	
		} else {
			std::cerr << "Invalid instruction opcode " << (unsigned int) opcode << std::endl;
			assert(false);
		}
	
		PC = PC + instruction::AM_sizes[instruction.addressing_mode];
	}
}

}
