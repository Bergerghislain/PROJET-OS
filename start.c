#include "cpu.h"
#include <inttypes.h>
#include "ecran.h"
#include "interruption.h"
#include "processus.h"
#include <string.h>
#include <stdio.h>


// programme proc1
// on annonce l'existence du processus
// puis on le termine
void proc1(void)
{
        ecrit_process();
        sleep(1);
}

// programme proc3
// processus test
void proc3(void){
        ecrit_process();
        sleep(1);
}

// programme proc2
// on annonce l'existence du processus
// on attend que proc1 termine
// on teste la creation d'un processus
// pour verifier que proc3 prend le pid de proc1 (zombie)
void proc2(void){
	ecrit_process();
	sleep(3);
	cree_processus(proc3, "proc3");
}

extern processus * active_process;

void kernel_start(void) {
  efface_ecran();

  // define clock interuption to call "traitant_IT_32" which will call "ordonnance"
  init_traitant_IT(32, traitant_IT_32);

  // set frequence to 50 hz
  set_clock(50);

  // set IRQ0 to unmasked (clock signal unmasked)
  masqueIRQ(0, 0);

  cree_processus(proc1, "proc1");
  cree_processus(proc2, "proc2");

  // define master process as idle and run it
  start();

  // on ne doit jamais sortir de kernel_start
  while (1) {
    // cette fonction arrete le processeur
    hlt();
  }
}
