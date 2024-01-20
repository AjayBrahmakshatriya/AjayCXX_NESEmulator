#include <cstring>
#include "pipeline.h"
#include "runtime.h"
#include "builder/builder_context.h"
#include "builder/builder_dynamic.h"
#include "blocks/c_code_generator.h"

namespace nes {

nes_program_t execution_context_t::get_program_ptr(nes_word offset) {
	if (program_map.find(offset) != program_map.end()) {
		return program_map[offset];
	}

	builder::builder_context context;
	context.dynamic_header_includes = "#include \"runtime.h\"\n";
	context.dynamic_compiler_flags = "-O3 -I runtime/ -L build -lnes_runtime -fPIC";

	auto func = (nes_program_t)(builder::compile_function_with_context(context, 
		nes::emulate_code, (nes::nes_byte*)main_memory, offset));

	program_map[offset] = func;
	
	return func;
	
	
}

int execution_context_t::attach_rom(rom_t &rom) {
	switch(rom.mapper_number) {
		case 0:
			memcpy(dmain_memory + 0x8000, rom.prg_rom, 16 * 1024);
			if (rom.prg_rom_size == 16 * 1024) {
				memcpy(dmain_memory + 0xC000, rom.prg_rom, 16 * 1024);
			} else {
				memcpy(dmain_memory + 0xC000, rom.prg_rom + 16 * 1024, 16 * 1024);
			}
			break;
		default:
			std::cerr << "Invalid or unsupported mapper number" << std::endl;
			return -1;

	}
	nes_byte prl = dmain_memory[0xfffc];
	nes_byte prh = dmain_memory[0xfffd];
	program_offset = prh << 8 | prl;
	return 0;
}

void execution_context_t::run_main_loop(void) {

	init_regs();

	do {
		auto func = get_program_ptr(program_offset);	
		assert(func != NULL);
		program_offset = func(dmain_memory);
		printf("Function execution returned = %x\n", (unsigned int) program_offset);

	} while (program_offset != 0);
		
}

}
