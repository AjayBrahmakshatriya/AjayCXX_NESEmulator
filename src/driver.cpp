#include <iostream>
#include "emulator.h"
#include "pipeline.h"
#include "runtime.h"
#include "loader.h"

int main(int argc, char* argv[]) {

	if (argc < 2) {
		printf("Usage: <%s> <ROM filename>\n", argv[0]);
		return 1;
	}

	nes::rom_t rom;
	rom.load_file(argv[1]);

	nes::execution_context_t execution_context;
	execution_context.attach_rom(rom);
	

	execution_context.run_main_loop();
		

	//printf("Regs: Acc = (%x), X = (%x), Y = (%x)\n", reg_acc, reg_x, reg_y);	

	return 0;
}
