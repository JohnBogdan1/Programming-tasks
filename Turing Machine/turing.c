
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
	int stare_urm; 
	char char_w, deplasare;
} Celula;

typedef int bool;
#define true 1

char* Turing(Celula **matrix, char* banda, int s, int sf){

	int i;

	// Variabila cursor este folosita pentru a  ne deplasa pe banda
	int cursor = 1;

	/* Variabila index primeste o valoare de la 0 la 2 
	in functie de caracterul citit de pe banda */
	int index;

	// Starea intiala este 0
	int stare_urmatoare = 0;

	// Caracterul scris si deplasarea
	char write_char, move_dir;
	
	while(true) {
	
		if (banda[cursor] == '0')
			index = 0;
		else if (banda[cursor] == '1')
			index = 1;
		else if (banda[cursor] == '#')
			index = 2;
		
		// Preiau informatiile din celula matricei de la starea curenta
		// Liniile matricei sunt corespunzatoare starilor
		// Coloanele matricei sunt corespunzatoare caracterului citit

		// Preiau caracterul care trebuie scris pe banda
		write_char = matrix[stare_urmatoare][index].char_w;

		// Preiau deplasarea
		move_dir = matrix[stare_urmatoare][index].deplasare;

		// Preiau starea urmatoare
		stare_urmatoare = matrix[stare_urmatoare][index].stare_urm;

		if(stare_urmatoare == -1)
			return "Eroare!";

		// Inlocuiesc caracterul de pe banda cu caracterul luat din matrice
		banda[cursor] = write_char;

		// Daca starea urmatoare este printre starile finale, returnez banda
		// Starile finale sunt ultimele sf stari
		for(i = s - sf; i <= s - 1; i++){
			if (stare_urmatoare == i) {
				return banda;
			}
		}
		
		// Ma misc la dreapta/stanga in functie de deplasare
		if(move_dir == 'R')
			cursor++;
		else if (move_dir == 'L')
			cursor--;
	}
	
}

int main(){

	char* banda;
	int s, sf, i, j;
	
	banda = malloc(100 * sizeof(char));
	FILE *input = NULL;
	
	// Declar o matrice de tip Celula
	// In ea voi avea celule care contin informatii de tip Celula
	Celula **mat_aux;

	input = fopen("date.in", "r");
	
	if (input == NULL) {
		printf("\n Cannot open file\n");
		exit(1);
	}
	
	// Citesc banda
	fscanf(input, "%s", banda);

	// Citesc numarul de stari si numarul de stari finale
	fscanf(input, "%d %d", &s, &sf);
	
	
	// Aloc dinamic matricea
	mat_aux = (Celula **) malloc((s - sf)* sizeof(Celula*));

	for (i = 0; i < s - sf; i++) {
		mat_aux[i] = (Celula *) malloc(3 * sizeof(Celula));
	}

	char b, c;
	int a;
	for (i = 0; i < s - sf; i++) {
		for (j = 0; j < 3; j++) {
			// Citesc primul caracter de pe linie si verific
			if(fscanf(input, " %d", &a) == 1) {
				// Daca e diferit de -1, citesc urmatoarele doua caractere de pe linie
				if(a != -1){
					fscanf(input, " %c %c", &b, &c);
				/* Daca primul caracter citit de pe linie e -1,
				 avem o stare nedefinita ,deci b si c sunt nedefinite(notate cu 'x')*/
				} else if (a == -1){
					b = 'x';
					c = 'x';
				}
			}
			// Pun in matrice informatiile citite
			mat_aux[i][j].stare_urm = a;
			mat_aux[i][j].char_w = b;
			mat_aux[i][j].deplasare = c;			
		}	
	}
	
	char* rezultat;
	
	rezultat = malloc(100 * sizeof(char));
	rezultat = Turing(mat_aux, banda, s, sf);
	
	
	FILE *output = NULL;
	
	output = fopen("date.out", "w");
	
	if (output == NULL) {
		printf("\n Cannot open file\n");
		exit(1);
	}
	
	// Scriu rezultatul
	fprintf(output, "%s", rezultat);
	
	fclose(input);
	fclose(output);

	// Eliberez memoria
	for (i = 0; i < abs(s - sf); i++) {
		free(mat_aux[i]);
	}
	free(mat_aux);

	free(banda);
	
	return 0;
}
