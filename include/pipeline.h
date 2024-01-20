#ifndef NES_PIPELINE_H
#define NES_PIPELINE_H

#include <map>
#include "emulator.h"
#include "loader.h"

namespace nes {

typedef nes::nes_word (* nes_program_t) (nes::nes_byte *);

struct execution_context_t {
	nes_word program_offset;
	nes_byte* main_memory;
	nes_byte* dmain_memory;

	execution_context_t() {	
		dmain_memory = (nes_byte*) calloc(0x10000, 1);
		main_memory = dmain_memory;
	}

	std::map<nes_word, nes_program_t> program_map;

	nes_program_t get_program_ptr(nes_word);	

	void run_main_loop();
	int attach_rom(rom_t&);
};


}


#endif
