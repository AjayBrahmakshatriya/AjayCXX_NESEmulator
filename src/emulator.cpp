#include "emulator.h"
#include "instructions.h"
#include <iostream>
namespace nes {

dnes_byte acc = builder::as_global("reg_acc");
dnes_byte xreg = builder::as_global("reg_x");
dnes_byte yreg = builder::as_global("reg_y");
dnes_byte sp = builder::as_global("reg_sp");
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
		case instruction::AM_A:
			return context.acc;
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
dnes_byte get_status(nes_context_t context) {
	dnes_byte status = context.sign_flag       	<< 7 | 
				context.overflow_flag   << 6 |
				1                       << 5 |
				context.break_flag      << 4 |
				context.decimal_flag    << 3 |
				context.interrupt_flag  << 2 |
				context.zero_flag       << 1 |
				context.carry_flag      << 0;
	return status;
}
void set_status(nes_context_t context, dnes_byte status) {
	context.sign_flag = 		(status >> 7) & 0x1;
	context.overflow_flag = 	(status >> 6) & 0x1;
	//discard = 			(status >> 5) & 0x1;
	context.break_flag = 		(status >> 4) & 0x1;
	context.decimal_flag = 		(status >> 3) & 0x1;
	context.interrupt_flag = 	(status >> 2) & 0x1;
	context.zero_flag = 		(status >> 7) & 0x1;
	context.carry_flag = 		(status >> 7) & 0x1;
}
static void store_byte (nes_context_t context, nes_byte* instruction_addr, instruction::nes_instruction_t instruction, 
		dnes_byte data) {
	instruction_addr += 1;
	nes_word addr;
	switch (instruction.addressing_mode) {
		case instruction::AM_A:
			context.acc = data;
			break;
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
	// The negativeness of the byte is checked by looking at the highest bit
	context.sign_flag = (output >> 7);
}
static void set_Z (nes_context_t context, dnes_byte &output) {
	context.zero_flag = (output == 0);
}

static int get_relative_offset(nes_byte d) {
	if (d & 0x80) return -(d & 0x7f);
	else return (d & 0x7f);
}

void sync_regs_out(nes_context_t context) {
	acc = context.acc;
	xreg = context.xreg;
	yreg = context.yreg;
	sp = context.sp;
	carry_flag = context.carry_flag;
	interrupt_flag = context.interrupt_flag;
	decimal_flag = context.decimal_flag;
	break_flag = context.break_flag;
	overflow_flag = context.overflow_flag;
	sign_flag = context.sign_flag;	
}
void sync_regs_in(nes_context_t context) {
	context.acc = acc;
	context.xreg = xreg;
	context.yreg = yreg;
	context.sp = sp;
	context.carry_flag = carry_flag;
	context.interrupt_flag = interrupt_flag;
	context.decimal_flag = decimal_flag;
	context.break_flag = break_flag;
	context.overflow_flag = overflow_flag;
	context.sign_flag = sign_flag;	
	
}

dnes_word emulate_code (nes_byte* main_memory, nes_word offset, dnes_byte_addr dmain_memory) {

	dnes_byte lacc;
	dnes_byte lxreg;
	dnes_byte lyreg;
	dnes_byte lsp;
	dnes_byte lcarry_flag;
	dnes_byte lzero_flag;
	dnes_byte linterrupt_flag;
	dnes_byte ldecimal_flag;
	dnes_byte lbreak_flag;
	dnes_byte loverflow_flag;
	dnes_byte lsign_flag;

	struct nes_context_t context (lacc, lxreg, lyreg, lsp, lcarry_flag, lzero_flag, linterrupt_flag, 
		ldecimal_flag, lbreak_flag, loverflow_flag, lsign_flag, main_memory, dmain_memory);	
	
	sync_regs_in(context);

	builder::static_var<nes_word> PC = offset;	

	while (1) {
		nes_byte opcode = main_memory[PC];
		instruction::nes_instruction_t instruction = instruction::nes_instructions[opcode];


		switch (instruction.opcode) {
			case instruction::ADC:
				{
					dnes_byte data = load_byte(context, &main_memory[PC], instruction);
					dnes_word res = (dnes_word) (context.acc) + data + context.carry_flag;
					context.acc = res | 0xff;
					context.carry_flag = !!(res >> 8);
					//if (context.carry_flag) context.overflow_flag = 1;
					context.overflow_flag = context.carry_flag;
					set_N(context, context.acc); 
					set_Z(context, context.acc); 
				}
				break;
			case instruction::AND:
				{
					dnes_byte data = load_byte(context, &main_memory[PC], instruction);
					context.acc = context.acc & data;
					set_N(context, context.acc); 
					set_Z(context, context.acc); 
				}
				break;
			case instruction::ASL:
				{
					dnes_word res = load_byte(context, &main_memory[PC], instruction);
					res = res << 1;
					dnes_byte out = res | 0xff;
					context.carry_flag = !!(res >> 8);
					store_byte(context, &main_memory[PC], instruction, out);
					set_N(context, out); 
					set_Z(context, out); 
				}
				break;
			case instruction::BCC:
				if (!context.carry_flag)  {
					nes_byte off = main_memory[PC+1];
					PC += get_relative_offset(main_memory[PC+1]);
					continue;
				}
				break;
			case instruction::BCS:
				if (context.carry_flag)  {
					nes_byte off = main_memory[PC+1];
					PC += get_relative_offset(main_memory[PC+1]);
					continue;
				}
				break;	
			case instruction::BEQ:
				if (context.zero_flag)  {
					nes_byte off = main_memory[PC+1];
					PC += get_relative_offset(main_memory[PC+1]);
					continue;
				}
				break;
			case instruction::BNE:
				if (!context.zero_flag)  {
					nes_byte off = main_memory[PC+1];
					PC += get_relative_offset(main_memory[PC+1]);
					continue;
				}
				break;
			case instruction::BPL:
				if (!context.sign_flag)  {
					nes_byte off = main_memory[PC+1];
					PC += get_relative_offset(main_memory[PC+1]);
					continue;
				}
				break;
			case instruction::BRK:
				{
					PC = PC + 1;
					// Store the PC
					nes_byte low = PC | 0xff;
					nes_byte high = PC >> 8;
					dmain_memory[context.sp    ] = high;
					dmain_memory[context.sp - 1] = low;
					context.sp = context.sp - 2;
					// Store the status
					dnes_byte status = get_status(context);
					dmain_memory[context.sp] = status;	
					context.sp = context.sp - 1;
					
					// Now we have to do an indirect jump
					dnes_byte addr_low =  dmain_memory[0xfffe];
					dnes_word addr_high = dmain_memory[0xffff];
					dnes_word addr = (addr_high << 8) | addr_low;

					// We cannot update the PC here, because we don't know what the value is
					// We will instead return this value at runtime and the emulator will trigger 
					// another compile cycle
					sync_regs_out(context);
					return addr;
				}
				break;
			case instruction::BVC:
				if (!context.overflow_flag)  {
					nes_byte off = main_memory[PC+1];
					PC += get_relative_offset(main_memory[PC+1]);
					continue;
				}
				break;
			case instruction::BVS:
				if (context.overflow_flag)  {
					nes_byte off = main_memory[PC+1];
					PC += get_relative_offset(main_memory[PC+1]);
					continue;
				}
				break;
			case instruction::CLC:
				context.carry_flag = 0;
				break;
			case instruction::CLD:
				context.decimal_flag = 0;
				break;
			case instruction::CLI:
				context.interrupt_flag = 0;
				break;
			case instruction::CLV:
				context.overflow_flag = 0;
				break;
			case instruction::CMP:
				{
					dnes_byte data = load_byte(context, &main_memory[PC], instruction);
					dnes_word res = context.acc;
					// Set the lowest bit in the second byte
					// before perfoming the subtraction
					res = res | 1 << 8;	
					res = res - data;
					//context.acc = res | 0xff;
					context.carry_flag = !!(res >> 8);					
					dnes_byte out = res | 0xff;
					set_N(context, out);
					set_Z(context, out);	
				}
				break;
			case instruction::CPX:
				{
					dnes_byte data = load_byte(context, &main_memory[PC], instruction);
					dnes_word res = context.xreg;
					// Set the lowest bit in the second byte
					// before perfoming the subtraction
					res = res | 1 << 8;	
					res = res - data;
					//context.acc = res | 0xff;
					context.carry_flag = !!(res >> 8);					
					dnes_byte out = res | 0xff;
					set_N(context, out);
					set_Z(context, out);	
				}
				break;
			case instruction::CPY:
				{
					dnes_byte data = load_byte(context, &main_memory[PC], instruction);
					dnes_word res = context.yreg;
					// Set the lowest bit in the second byte
					// before perfoming the subtraction
					res = res | 1 << 8;	
					res = res - data;
					//context.acc = res | 0xff;
					context.carry_flag = !!(res >> 8);					
					dnes_byte out = res | 0xff;
					set_N(context, out);
					set_Z(context, out);	
				}
				break;
			case instruction::DEC:
				{
					dnes_byte data = load_byte(context, &main_memory[PC], instruction);
					data = data - 1;
					set_N(context, data);
					set_Z(context, data);
					store_byte(context, &main_memory[PC], instruction, data);
				}
				break;
			case instruction::DEX:
				context.xreg = context.xreg - 1;
				set_N(context, context.xreg);
				set_Z(context, context.xreg);
				break;
			case instruction::DEY:
				context.yreg = context.yreg - 1;
				set_N(context, context.yreg);
				set_Z(context, context.yreg);
				break;
			case instruction::EOR:
				{
					dnes_byte data = load_byte(context, &main_memory[PC], instruction);
					context.acc = context.acc ^ data;
					set_N(context, context.acc);
					set_Z(context, context.acc);
				}
				break;
			case instruction::EXT:
				// Returning 0 indicates to the interpreter that we are done
				// Sync the regs out in case the calling program wants to read the registers
				sync_regs_out(context);
				return 0x0000u;
			case instruction::INC:
				{
					dnes_byte data = load_byte(context, &main_memory[PC], instruction);
					data = data + 1;
					set_N(context, data);
					set_Z(context, data);
					store_byte(context, &main_memory[PC], instruction, data);
				}
				break;
			case instruction::INX:
				context.xreg = context.xreg + 1;
				set_N(context, context.xreg);
				set_Z(context, context.xreg);
				break;
			case instruction::INY:
				context.yreg = context.yreg + 1;
				set_N(context, context.yreg);
				set_Z(context, context.yreg);
				break;
			case instruction::JMP:
				{
					if (instruction.addressing_mode == instruction::AM_abs) {
						nes_word addr = (main_memory[PC + 2]) << 8 | main_memory[PC + 1];
						PC = addr;
					} else {
						// indirect jump		
						nes_word addr = (main_memory[PC + 2]) << 8 | main_memory[PC + 1];
						dnes_byte low =  dmain_memory[addr];
						dnes_word high = dmain_memory[addr + 1];
						dnes_word ret_addr = (high << 8) | low;
						// Before we jump we need to sync all the registers
						sync_regs_out(context);
						// Return the value to jump to
						return ret_addr;
					}
					continue;
				}
				break;			
			case instruction::JSR:
				{
					// the only addressing mode is absolute
					nes_word addr = (main_memory[PC + 2]) << 8 | main_memory[PC + 1];
					// the address pushed is the next instruction - 1 
					// RTS also takes care of this
					PC = PC + 2;
					nes_byte low = PC | 0xff;
					nes_byte high = PC >> 8;
					dmain_memory[context.sp    ] = high;
					dmain_memory[context.sp - 1] = low;
					context.sp = context.sp - 2;
					PC = addr;
					continue;
				}
				break;
			case instruction::LDA:
				{
					dnes_byte data = load_byte(context, &main_memory[PC], instruction);
					context.acc = data;
					set_N(context, data); 
					set_Z(context, data); 
				}
				break;
			case instruction::LDX:
				{
					dnes_byte data = load_byte(context, &main_memory[PC], instruction);
					context.xreg = data;
					set_N(context, data); 
					set_Z(context, data); 
				}
				break;
			case instruction::LDY:
				{
					dnes_byte data = load_byte(context, &main_memory[PC], instruction);
					context.yreg = data;
					set_N(context, data); 
					set_Z(context, data); 
				}
				break;
			case instruction::LSR:
				{
					dnes_byte res = load_byte(context, &main_memory[PC], instruction);
					context.carry_flag = res & 0x1;
					res = res >> 1;
					store_byte(context, &main_memory[PC], instruction, res);
					context.sign_flag = 0;
					set_Z(context, res); 
				}
				break;
			case instruction::NOP:
				break;
			case instruction::ORA:
				{
					dnes_byte data = load_byte(context, &main_memory[PC], instruction);
					context.acc = context.acc | data;
					set_Z(context, context.acc);
					set_N(context, context.acc);
				}
				break;
			case instruction::PHA:
				dmain_memory[context.sp] = context.acc;
				context.sp = context.sp - 1;
				break;
			case instruction::PHP:
				{
					dmain_memory[context.sp] = get_status(context);
					context.sp = context.sp - 1;
				}
				break;
			case instruction::PLA:
				context.sp = context.sp + 1;
				context.acc = dmain_memory[context.sp];
				break;	
			case instruction::PLP:
				{
					context.sp = context.sp + 1;
					dnes_byte status = dmain_memory[context.sp];
					set_status(context, status);
				}
				break;	
			case instruction::ROL:
				{
					dnes_word data = load_byte(context, &main_memory[PC], instruction);
					data = data << 1;	
					data = (data | context.carry_flag);	
					context.carry_flag = !!(data >> 8);
					dnes_byte out = data & 0xff;
					store_byte(context, &main_memory[PC], instruction, out);
					set_N(context, out);	
					set_Z(context, out);	
				}
				break;
			case instruction::ROR:
				{
					dnes_word data = load_byte(context, &main_memory[PC], instruction);
					data = data | (context.carry_flag << 8);
					context.carry_flag = data & 0x1;
					data = data >> 1;	
					dnes_byte out = data & 0xff;
					store_byte(context, &main_memory[PC], instruction, out);
					set_N(context, out);	
					set_Z(context, out);	
				}
				break;
			case instruction::RTI:
				{
					dnes_byte status = dmain_memory[context.sp];
					context.sp = context.sp + 1;	
					set_status(context, status);
					dnes_byte low =  dmain_memory[context.sp];
					dnes_word high = dmain_memory[context.sp + 1];
					context.sp = context.sp + 2;
					dnes_word addr = (high << 8) | low;
					sync_regs_out(context);
					return addr;	
				}
				break;
			case instruction::RTS:
				{
					// This needs to be implemented like JMP (Operand)
					// because the jump target is read from memory
					// We can also maintain a stack at compile time 
					// and use that to return, however we still have to assert that the 
					// return address matches the value on the stack
					
					// load the value from the stack
					dnes_byte low = dmain_memory[context.sp];
					dnes_word high = dmain_memory[context.sp + 1];
					context.sp = context.sp + 2;	
					dnes_word addr = (high << 8) | low;
					addr = addr + 1;
					sync_regs_out(context);
					return addr;
				}
				break;
			case instruction::SBC:
				{
					dnes_byte data = load_byte(context, &main_memory[PC], instruction);
					dnes_word res = context.acc;
					res = res | 1 << 8;	
					res = res - data - (1 - context.carry_flag);
					context.acc = res | 0xff;
					context.carry_flag = !!(res >> 8);

					set_N(context, context.acc); 
					set_Z(context, context.acc); 
					//if (!context.carry_flag) context.overflow_flag = 1;
					context.overflow_flag = !context.carry_flag;
				}
			case instruction::SEC:
				context.carry_flag = 1;
				break;
			case instruction::SED:
				context.decimal_flag = 1;
				break;
			case instruction::SEI:
				context.interrupt_flag = 1;
				break;
			case instruction::STA:
				store_byte(context, &main_memory[PC], instruction, context.acc);
				break;
			case instruction::STX:
				store_byte(context, &main_memory[PC], instruction, context.xreg);
				break;
			case instruction::STY:
				store_byte(context, &main_memory[PC], instruction, context.yreg);
				break;
			case instruction::TAX:
				context.xreg = context.acc;
				set_N(context, context.xreg);
				set_Z(context, context.xreg);	
				break;
			case instruction::TAY:
				context.yreg = context.acc;
				set_N(context, context.yreg);
				set_Z(context, context.yreg);	
				break;
			case instruction::TSX:
				context.xreg = context.sp;
				set_N(context, context.xreg);
				set_Z(context, context.xreg);	
				break;
			case instruction::TXA:
				context.acc = context.xreg;
				set_N(context, context.acc);
				set_Z(context, context.acc);	
				break;	
			case instruction::TXS:
				context.sp = context.xreg;
				break;
			case instruction::TYA:
				context.acc = context.yreg;
				set_N(context, context.acc);
				set_Z(context, context.acc);	
				break;			
			default:
				std::cerr << "Invalid instruction opcode " << (unsigned int) opcode << std::endl;
				assert(false);		
			
		}


		PC = PC + instruction::AM_sizes[instruction.addressing_mode];
	}
}

}
