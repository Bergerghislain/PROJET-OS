#ifndef ECRAN_H_
#define ECRAN_H_

#include "inttypes.h"

uint16_t *ptr_mem(uint32_t lig, uint32_t col);
void ecrit_car(uint32_t lig, uint32_t col, char c);
void efface_ecran(void);
uint16_t get_curseur(void);
void place_curseur(uint32_t lig, uint32_t col);
void traite_car(char c);
void defilement();
void console_putbytes(const char * str, int len);
void ecrit_temps(uint32_t secondes);

#endif
