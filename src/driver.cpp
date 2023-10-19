#include <iostream>
#include "builder/builder_context.h"
#include "builder/builder_dynamic.h"
#include "blocks/c_code_generator.h"
#include "emulator.h"
#include "runtime.h"

int main(int argc, char* argv[]) {
	
	nes::nes_byte program[] = {
		0xa9, 0xab,
		0xaa, 
		0xe8,
		0xff,	
	};

	nes::nes_word offset = 0;
	
	builder::builder_context context;
	context.dynamic_header_includes = "#include \"runtime.h\"\n";
	context.dynamic_compiler_flags = "-O3 -I runtime/ -L build -lnes_runtime -fPIC";

	auto func = (nes::nes_word (*) (nes::nes_byte *))(builder::compile_function_with_context(context, nes::emulate_code, 
		(nes::nes_byte*)program, offset));

	//printf("Function pointer returned = %p\n", (char*) func);

	unsigned char* main_memory = (unsigned char*) calloc(0x10000, 1);

	init_regs();
	auto ret = func(main_memory);
	
	printf("Function execution returned = %x\n", (unsigned int) ret);

	printf("Regs: Acc = (%x), X = (%x), Y = (%x)\n", reg_acc, reg_x, reg_y);	
	
	return 0;
}
