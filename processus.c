#include "inttypes.h"
#include <stdio.h>
#include <stdbool.h>
#include <tinyalloc.h>
#include <string.h>
#include "cpu.h"
#include "ecran.h"
#include "interruption.h"

#define N 8
#define STACK_SIZE 512
#define END_STACK STACK_SIZE - 1

// les differents etates pour un processus
enum states {elu,activable,endormi,zombie};


// structure d'un processus
struct processus{
	int pid;
	char name[20];
	enum states state;
        uint32_t register_save[5];
	uint32_t stack[STACK_SIZE];
	struct processus * suiv;
	uint32_t wake_time;
};

typedef struct processus processus;

// changement de contexte
extern void ctx_sw(uint32_t * old_context, uint32_t * new_context);


processus * active_process = NULL;

processus * first_activatable_process = NULL;
processus * last_activatable_process = NULL;

processus * first_sleeping_process = NULL;

processus * first_zombie_process = NULL;

int32_t next_pid = 1;


char * getStateString(enum states state){
	switch(state){
		case elu: return "elu";
		case activable: return "activable";
		case endormi: return "endormi";
		case zombie: return "zombie";
	}
	return "";
}

void print_state_by_queue(processus * iter){
	char str[22];
	while(iter != NULL){
                sprintf(str, "%s:%s ", iter->name, getStateString(iter->state));
                console_putbytes(str, strlen(str));
		iter = iter->suiv;
        }

}

void affiche_etats(void){
	uint16_t saved_curser_pos = get_curseur();
	place_curseur(0,0);
	print_state_by_queue(first_activatable_process);
	print_state_by_queue(first_sleeping_process);
	print_state_by_queue(first_zombie_process);
	place_curseur(saved_curser_pos>>8, saved_curser_pos & 0xFF);
}


// insert p in activatable queue (to be executed)
void insert_processus_in_activatable_queue(processus * p){
	p->state = activable;
	p->suiv = NULL;
	if(first_activatable_process == NULL){
		first_activatable_process = p;
		last_activatable_process = p;
		return;
	}
	last_activatable_process->suiv = p;
	last_activatable_process = p;
}


// extract next process from activatable queue to be executed
processus * extract_processus_from_activatable_head(){
	processus * process = first_activatable_process;
	if(first_activatable_process != NULL) {
		first_activatable_process->state = elu;
		if(first_activatable_process == last_activatable_process)
			last_activatable_process = NULL;
		first_activatable_process = first_activatable_process->suiv;
	}
	return process;
}

// put a process in sleeping mode
// (you have to set wake_time yourself)
void insert_processus_in_sleep_queue(processus * p){
	// set process state and wake time
	p->state = endormi;

	processus * iter = first_sleeping_process;
	processus * prev = NULL;

	while ( iter != NULL && iter->wake_time < p->wake_time ){
		prev = iter;
		iter = iter->suiv;
	}

	if (prev == NULL){
		p->suiv = first_sleeping_process;
		first_sleeping_process = p;
	} else {
		p->suiv = iter;
		prev->suiv = p;
	}
}

// check for process to be awake and if so wake them
void wake_processus_from_sleeping_queue(){
	processus * iter = first_sleeping_process;
	if(iter == NULL) return;

	processus * temp;
	// on recupere le temps depuis le démarrage du systeme
	uint32_t current_time = get_awake_time();

	while(iter !=  NULL && iter->wake_time <= current_time){
		temp = iter->suiv;
		insert_processus_in_activatable_queue(iter);
		iter = temp;
	}

	// if iter is NULL then sleeping queue is null so empty;
	first_sleeping_process = iter;
}


// insert processus p in zombie queue
void insert_processus_in_zombie_queue(processus * p){
	active_process->suiv = first_zombie_process;
        first_zombie_process = active_process;
}

processus * extract_processus_from_zombie_queue(){
	processus * p = first_zombie_process;
	first_zombie_process = NULL;
	if(p != NULL)
		first_zombie_process = p->suiv;
	return p;
}


void set_processus_parameter(processus * p, int32_t pid, char * name, enum states state){
	p->pid = pid;
	strcpy(p->name, name);
	p->state = state;
}


// fonction qui alloue dynamiquement un processus
processus * alloc_processus()
{
	return malloc(sizeof(processus));
}

// switch de processus
void ordonnance(void){

        if(active_process->state == endormi)
                insert_processus_in_sleep_queue(active_process);

        else if(active_process->state == zombie)
                insert_processus_in_zombie_queue(active_process);

        else
                insert_processus_in_activatable_queue(active_process);

        processus * old_process = active_process;
        // check from sleeping process to be wake and if so wake them
        wake_processus_from_sleeping_queue();
        active_process = extract_processus_from_activatable_head();

        // changement contexte
        ctx_sw( old_process->register_save,
                active_process->register_save);
}

// termine un processus
void fin_processus(void){
        active_process->state = zombie;
        ordonnance();
}

// cree un processus et assigne une fonction à celui-ci
int32_t cree_processus(void (*code)(void), char * nom){

	processus * p = extract_processus_from_zombie_queue();
	if(p == NULL){
		// allocation processus
		p = alloc_processus();
		set_processus_parameter(p, next_pid, nom, activable);
		next_pid++;
	} else{
		set_processus_parameter(p, p->pid, nom, activable);
	}

	// assignation fonction
	p->stack[END_STACK-1] = (uint32_t) code;
	p->register_save[1] = (uint32_t) &p->stack[END_STACK-1];

	p->stack[END_STACK] = (uint32_t) fin_processus;

	insert_processus_in_activatable_queue(p);
	return p->pid;
}

// retourne le pid du processus actif
int32_t mon_pid(void){
        return active_process->pid;
}

// retourne le nom du processus actif
char * mon_nom(void){
        return active_process->name;
}


// sleep current process for nbr_secs
void sleep(uint32_t nbr_secs){
	if(active_process->pid == 0) return;
        active_process->state = endormi;
        active_process->wake_time = get_awake_time() + nbr_secs;
        ordonnance();
}

// programme idle
void idle(void)
{
        for(;;){
                sti(); // enable extern interruptions
                hlt(); // sleep and wait for new interruption
                cli(); // disable extern interruptions
        }
}

void start(){
	active_process = alloc_processus();
	set_processus_parameter(active_process, 0, "idle", elu);
	ecrit_temps(0);
	idle();
}


void ecrit_process(){
	char str[20];
	sprintf(str, "[%s] pid = %i\n", mon_nom(), mon_pid());
	console_putbytes(str,strlen(str));
}

/*
void remove_processus(struct processus proc){
	//free(proc.name);
	//free(proc.register_save);
	free(proc.stack);
}
*/
