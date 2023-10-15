#include <iostream>
#include "builder/builder_context.h"
#include "blocks/c_code_generator.h"
#include "emulator.h"

int main(int argc, char* argv[]) {
/*	
	nes::nes_byte program[] = 	{
					0xa9, 0xab, 
					0xad, 0xff, 0x00,	
					0xa5, 0xf2,
					0xbd, 0xf2, 0x03,
					0xb9, 0xf2, 0x03,
					0xb5, 0xf2,
					0xa1, 0xf5,
					0xb1, 0xf3,
					0x80, 0xab, 0x00,
					0xff};
*/
/*
	nes::nes_byte program[] = {
		0xa9, 0xab,
		0xa9, 0xa3,	
		0xd0, 0x82,
		0xd0, 0x86,
		0xff
	};
*/
/*
	// while (x_reg != 0) {dest[X_reg] = src[X_reg]; X_reg--;}
	// dest - 0x80
	// src - 0x70
	nes::nes_byte program[] = {
		// LDA 0x70, X	
		// STA 0x80, X
		// DECX
		// BNE - LDA
		0xb5, 0x70,
		0x95, 0x80,
		0xca, 
		0xd0, 0x85,
		0xff
	};
*/
	// while (x_reg != 0) {dest[X_reg] = 'a'; X_reg--;}
	// dest - 0x80
	nes::nes_byte program[] = {
		// LDA 0x70, X	
		// STA 0x80, X
		// DECX
		// BNE - LDA
		0xa9, 'a',
		0x95, 0x80,
		0xca, 
		0xd0, 0x83,
		0xff
	};
	nes::nes_word offset = 0;
	
	builder::builder_context context;

	auto ast = context.extract_function_ast(nes::emulate_code, "run_code", (nes::nes_byte*)program, offset);

	std::cout << "#include \"runtime.h\"" << std::endl;

	block::c_code_generator::generate_code(ast, std::cout, 0);
	
	return 0;
}
