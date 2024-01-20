#ifndef RUNTIME_H
#define RUNTIME_H

#include <stdio.h>

#ifdef __cplusplus 
extern "C" {
#endif

typedef unsigned short nes_word;
typedef unsigned char nes_byte;

extern nes_byte reg_acc;
extern nes_byte reg_x;
extern nes_byte reg_y;
extern nes_byte reg_sp;
extern nes_byte reg_flag_c;
extern nes_byte reg_flag_z;
extern nes_byte reg_flag_i;
extern nes_byte reg_flag_d;
extern nes_byte reg_flag_b;
extern nes_byte reg_flag_v;
extern nes_byte reg_flag_s;

void init_regs (void);

void runtime_unreachable(const char*);

#ifdef __cplusplus 
}
#endif

#endif
