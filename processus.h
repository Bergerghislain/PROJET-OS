#ifndef PROCESSUS_H_
#define PROCESSUS_H_

#include "inttypes.h"

// les etats possibles pour un processus
enum states {elu,activable};

// structure d'un processus
struct processus
{
	int pid;
        char name[20];
        enum states state;
        uint32_t register_save[5];
        uint32_t stack[512];
};

typedef struct processus processus;

void affiche_etats(void);

// fonction qui alloue dynamiquement un processus
struct processus * define_processus(    int _pid,
                                        char * _name,
                                        enum states _state);


// termine processus
void fin_processus();

// creation processus
int32_t cree_processus(void (*code)(void), char * nom);


// changement de contexte
extern void ctx_sw(uint32_t * old_context, uint32_t * new_context);

// retourne le pid du processus actif
int32_t mon_pid(void);

// retourne le nom du processus actif
char * mon_nom(void);

// print process pid and name
void ecrit_process(void);

// switch de processus
void ordonnance(void);

// sleep for uint32_t seconds
void sleep(uint32_t);

// define idle as master process and run it
void start();
#endif
