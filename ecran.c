#include <inttypes.h>
#include <cpu.h>
#include <string.h>
#include <stdio.h>

#define HEIGHT 25
#define LENGTH 80

uint16_t *ptr_mem(uint32_t lig, uint32_t col){
        uint16_t * address = (uint16_t *) (0xB8000 + 2 * (lig*LENGTH+col));
        return address;
}


// global var to save curser location
uint8_t g_lig;
uint8_t g_col;

// return curser pos:
// 8 low bits are column
// 8 high bits are line
uint16_t get_curseur(){
	return g_lig << 8 | g_col;
}

void place_curseur(uint32_t lig, uint32_t col){
	// update curser location
        g_lig = lig;
        g_col = col;

	// calcul pos
        int16_t pos = col + lig * 80;

	// send 8 lower bits of pos
	outb(0x0f, 0x3d4);
        outb((uint8_t) pos, 0x3d5);
        // send 8 upper bits of pos
	outb(0x0e, 0x3d4);
        outb((uint8_t) (pos >> 8), 0x3d5);
}

void ecrit_car(uint32_t lig, uint32_t col, char c){
        // calcul adresse
	uint16_t * address = ptr_mem(lig, col);
        // ecriture du caractère à l'adresse obtenue
	// 0x0f pour écrire en blanc
	*address = (0x0f00 + c);
}

void efface_ligne(uint8_t ligne){
	for(uint8_t j = 0; j < 80; j++){
		ecrit_car(ligne, j, ' ');
	}
}

void efface_ecran(void){
	for(uint8_t i = 0; i < 25; i++){
		efface_ligne(i);
	}
	place_curseur(1,0);
}

void copie_ligne(uint8_t destination_line, uint8_t source_line){
	memmove((uint8_t *) (0xB8000+destination_line*LENGTH*2),
		(uint8_t *) (0xB8000+source_line*LENGTH*2),
		LENGTH*2);
}


void defilement(){
        // le cast fait le x2 automatiquement vu que c'est un uint16_t
        /*
	for(uint16_t * i = (uint16_t *) 0xB8000; i <= (uint16_t *) (0xB8000 + 24*80); i += 80){
                memmove(i-80, i, 160);
        }
	*/
	for(uint8_t i = 2; i < HEIGHT; i++){
		copie_ligne(i-1, i);
	}
        place_curseur(g_lig-1,g_col);
	efface_ligne(HEIGHT-1);
}

void traite_car(char c){
	if((c <= 126) && (c >= 32)){
		ecrit_car(g_lig, g_col, c);
		g_col++;
		if(g_col==80){
			g_col = 0;
			g_lig++;
			if(g_lig == 25) defilement();
		}
		place_curseur(g_lig, g_col);
		return;
	}
	// special char
	switch(c){
		case 8:
			// char \b
			if(g_col != 0){
				g_col -= 1;
				place_curseur(g_lig, g_col);
			}
			break;
		case 9:
			// char \t
			g_col = 8*(g_col/8+1);
			if(g_col > 79) g_col = 79;
			place_curseur(g_lig, g_col);
			break;
		case 10:
			// char \n
			place_curseur(g_lig+1, 0);
			if(g_lig == 25) defilement();
			break;
		case 12:
			// char \f
			efface_ecran();
			place_curseur(0,0);
			break;
		case 13:
			// char \r
			place_curseur(g_lig, 0);
			break;
		default:
			return;
	}
}


void console_putbytes(const char * str, int len){
    for(int i = 0; i < len; i++) traite_car(str[i]);
}



void ecrit_temps(uint32_t secondes){
	efface_ligne(0);
	// calcul de l'heure
	uint8_t heures = secondes / 3600;
	secondes -= heures*3600;
	uint8_t minutes = secondes / 60;
	secondes -= minutes*60;

	// sauvegarde position curseur
	uint8_t col = g_col;
	uint8_t lig = g_lig;

	// affichage de l'heure
  	char temps[15];
  	sprintf(temps, "%02dh%02dm%02ds", heures, minutes, (uint8_t)secondes);
	place_curseur(0,LENGTH-10);
	console_putbytes(temps,10);

	// on restore la position du curseur originale
	place_curseur(lig,col);
}
