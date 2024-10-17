#ifndef INTERRUPTION_H_
#define INTERRUPTION_H_

#include "inttypes.h"
#include <stdbool.h>

void init_traitant_IT(uint32_t num_IT, void (*traitant)(void));
uint32_t get_awake_time();
void tic_PIT(void);
void masqueIRQ(uint32_t num_IRQ, bool masque);
void set_clock(uint32_t frequence);
extern void traitant_IT_32();

#endif
