#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include "loader.h"
#include <iostream>
#include <assert.h>
#include <sys/mman.h>
namespace nes {

static size_t get_file_size(std::string filename) {
	FILE* file = fopen(filename.c_str(), "r");
	assert(file != NULL);
	fseek(file, 0, SEEK_END);
	size_t file_size = ftell(file);
	fclose(file);
	return file_size;
}

int rom_t::parse_file(void) {
	assert(file_mapping != NULL);
	if (!(file_mapping[0] == 'N' && file_mapping[1] == 'E' && file_mapping[2] == 'S' && file_mapping[3] == 0x1a)) {
		std::cerr << "Invalid NES file format" << std::endl;
		return -1;
	}	
	prg_rom_size = file_mapping[4] * 16 * 1024;		
	chr_rom_size = file_mapping[5] * 8 * 1024;
	flag6 = file_mapping[6];
	flag7 = file_mapping[7];

	mapper_number = flag6 >> 4;
	mapper_number = mapper_number | (flag7 & 0xf0);
	assert(mapper_number == 0);

	prg_rom = file_mapping + 16;
	chr_rom = file_mapping + 16 + prg_rom_size;
	return 0;
}


int rom_t::load_file(std::string filename) {
	
	size_t file_size = get_file_size(filename);

	file_fd = open(filename.c_str(), O_RDONLY, 0);
	assert(file_fd >= 0);
	
	file_mapping = (unsigned char*)mmap(NULL, file_size, PROT_READ, MAP_SHARED, file_fd, 0);
	assert(file_mapping != NULL);

	int ret = parse_file();	
	if (ret != 0) return ret;

	return 0;	
}

}
