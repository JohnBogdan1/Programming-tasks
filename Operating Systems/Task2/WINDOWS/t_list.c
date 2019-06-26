/**
 * Operating Systems 2017 - Assignment 4
 *
 * Name, Group
 * Ion Bogdan-Ionut, 332CB
 *
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "t_list.h"
#include "so_scheduler.h"

/* add a node to the end of the linked list */
void t_list_add(ATListG list, void *data)
{

	TListG aux = NULL;
	TListG p = NULL;

	aux = (TList *) malloc(sizeof(TList));

	if (!aux) {
		perror("\nCan't alloc memory.");
		exit(EXIT_FAILURE);
	}

	aux->info = data;
	aux->urm = NULL;

	if ((*list) == NULL) {
		*list = aux;
		return;
	}

	p = *list;

	while (p->urm != NULL)
		p = p->urm;

	p->urm = aux;

}

/* destroy the linked list */
void t_list_destroy(ATListG list)
{
	TListG p = NULL, aux = NULL;

	p = *list;

	while (p != NULL) {

		aux = p;

		p = p->urm;

		free(aux->info);
		free(aux);
	}

	*list = NULL;
}
