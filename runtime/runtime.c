#include "runtime.h"
#include <stdio.h>
#include <stdlib.h>

nes_byte reg_acc;
nes_byte reg_x;
nes_byte reg_y;
nes_byte reg_sp;
nes_byte reg_flag_c;
nes_byte reg_flag_z;
nes_byte reg_flag_i;
nes_byte reg_flag_d;
nes_byte reg_flag_b;
nes_byte reg_flag_v;
nes_byte reg_flag_s;


void init_regs (void) {
	reg_acc = 0;	
	reg_x = 0;
	reg_y = 0;
	reg_sp = 0xfd;
	// Rest of the registers are uinitialized
}

void runtime_unreachable(const char* msg) {
	printf("Execution stopped with: %s\n", msg);
	exit(-1);
}

