#ifndef NES_LOADER_H
#define NES_LOADER_H
#include <string>
#include "emulator.h"
namespace nes {

struct rom_t {
	unsigned char* file_mapping;
	size_t prg_rom_size;
	size_t chr_rom_size;
	unsigned char flag6;
	unsigned char flag7;
	nes_byte* prg_rom;
	nes_byte* chr_rom;
	unsigned char mapper_number;

	/* internal fields */
	int file_fd;

	int parse_file(void);
	int load_file(std::string filename);
};

}

#endif
