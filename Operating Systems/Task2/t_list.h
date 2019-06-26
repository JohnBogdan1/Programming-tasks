/**
 * Operating Systems 2017 - Assignment 4
 *
 * Name, Group
 * Ion Bogdan-Ionut, 332CB
 *
 */

#include <stdio.h>
#include <stdlib.h>

/*
 * linked list structure used to keep all the threads
 * created during the schedule
 */
typedef struct cell_l {
	struct cell_l *urm;
	void *info;
} TList, *TListG, **ATListG;

void t_list_add(ATListG list, void *data);
void t_list_destroy(ATListG list);
