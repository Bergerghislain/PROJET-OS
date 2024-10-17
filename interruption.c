#include "cpu.h"
#include "inttypes.h"
#include "segment.h"
#include <stdbool.h>
#include "processus.h"
#include "ecran.h"

void init_traitant_IT(uint32_t num_IT, void (*traitant)(void)) {
  uint32_t adresse_32 = (uint32_t) traitant;
  uint16_t temp = (uint16_t) adresse_32;
  uint32_t *entree = (uint32_t *)(0x1000 + 2 * num_IT * 4);
  *entree = KERNEL_CS << 16 | temp;
  temp = (uint16_t) (adresse_32 >> 16);
  *(entree + 1) = temp << 16 | 0x8E00;
}

uint32_t secondes = 0;
uint8_t compteur = 50;

uint32_t get_awake_time(){
	return secondes;
}

void tic_PIT(void) {
  // signalement interruption
  outb(0x20, 0x20);
  compteur--;
  if (compteur == 0){
  	// il y a eu 1 seconde écoulée
  	// on incremente donc secondes
  	// et on reintialise le compteur
  	secondes++;
  	compteur = 50;
	ecrit_temps(secondes);
	affiche_etats();
  }
  ordonnance();
}

void masqueIRQ(uint32_t num_IRQ, bool masque) {
  uint8_t current_mask = inb(0x21);
  if (masque)
    current_mask = current_mask | 1 << num_IRQ;
  else
    current_mask = current_mask & ~( 1 << num_IRQ);
    //current_mask = 0xfe;
  outb(current_mask, 0x21);
}

void set_clock(uint32_t frequence) {
  if (frequence < 20)
    return; // ça ne tient pas dans 16 bit
  outb(0x34, 0x43);
  uint16_t valeur = 0x1234DD / frequence;
  outb(valeur & 0xFF, 0x40);
  outb(valeur >> 8, 0x40);
}

//extern void traitant_IT_32();
