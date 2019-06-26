/**
 * Operating Systems 2017 - Assignment 4
 *
 * Name, Group
 * Ion Bogdan-Ionut, 332CB
 *
 */

#include <stdio.h>
#include <stdlib.h>

/* linked list structure */
typedef struct cell {
	struct cell *urm;
	void *info;
} TCellG, *TLG, **ALG;

/* priority table structure */
typedef struct {
	unsigned int size;
	TLG *v;
} TP;

/* functions */
TP *priority_table_create(unsigned int size);
void DistrugeLG(ALG aL);
void DistrugeTP(TP **atd);
void priority_table_insert(TP *atd, void *info, int priority);
void priority_table_remove(TP *atd, int priority);
TLG get_head(TP *atp);
