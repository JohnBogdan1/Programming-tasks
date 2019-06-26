/**
 * Operating Systems 2017 - Assignment 4
 *
 * Name, Group
 * Ion Bogdan-Ionut, 332CB
 *
 */

#include "priority_table.h"
#include "so_scheduler.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* create the priority table */
TP *priority_table_create(unsigned int size)
{
	TP *atp = NULL;

	atp = (TP *) malloc(sizeof(TP));
	if (atp == NULL)
		return NULL;

	/* alloc the linked lists vector */
	atp->v = calloc(size, sizeof(TLG));
	if (atp->v == NULL) {
		free(atp);
		return NULL;
	}

	atp->size = size;

	return atp;
}

/* insert in the table an element at the given priority position */
void priority_table_insert(TP *atp, void *info, int priority)
{
	int index = priority;
	TLG aux = NULL;
	ALG aL = NULL;

	if (priority > SO_MAX_PRIO)
		return;

	aL = atp->v + index;

	/* go to the end */
	while (*aL != NULL)
		aL = &(*aL)->urm;

	/* create an aux cell to put the info */
	aux = (TCellG *) malloc(sizeof(TCellG));

	if (!aux) {
		perror("\nCan't alloc memory.");
		exit(EXIT_FAILURE);
	}

	/* set the info and the next link to NULL */
	aux->info = info;
	aux->urm = NULL;

	/* put the new cell at the end */
	aux->urm = *aL;
	*aL = aux;
}

/* remove the first node at the given priority positon */
void priority_table_remove(TP *atp, int priority)
{
	TLG node = NULL, prevnode = NULL;
	int index = priority;

	/* get the first node */
	node = atp->v[index];

	if (node != NULL)
		atp->v[index] = node->urm;

}

/*
 * get the head of the priority table
 * the head is located at the biggest current priority
 * on the first node
 */
TLG get_head(TP *atp)
{

	int p;
	TLG node;

	if (!atp)
		return NULL;

	for (p = (atp->size - 1); p >= 0; p--) {
		node = atp->v[p];

		if (node != NULL)
			return node;
	}

	return NULL;


}

/* destroy a linked list */
void DistrugeLG(ALG aL)
{
	TLG aux = NULL, p = NULL;

	p = *aL;

	while (p) {

		/* last position of the node */
		aux = p;

		/* go to the next node */
		p = p->urm;

		/* free the memory */
		free(aux->info);
		free(aux);
	}

	*aL = NULL;
}

/* destroy the whole priority table */
void DistrugeTP(TP **atp)
{
	size_t n;
	TLG L = NULL;

	if (*atp == NULL)
		return;

	/* cross the vector and destroy every list */
	for (n = 0; n < (*atp)->size; n++) {
		L = (*atp)->v[n];
		DistrugeLG(&L);
	}

	/* free the vector and the table memory */
	free((*atp)->v);
	free(*atp);
	*atp = NULL;
}
