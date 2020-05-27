#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stddef.h>

#include "list.h"

struct List *list_new()
{
	struct List *list = malloc(sizeof(struct List));

	if (list == NULL)
		exit(EXIT_FAILURE);

	list->size = 0;
	list->head = NULL;
	list->tail = NULL;

	return list;
}

void list_push(struct List *list, void *data)
{
	struct ListNode *node = malloc(sizeof(struct ListNode));
	if (node == NULL)
		exit(EXIT_FAILURE);

	node->data = data;
	node->next = NULL;

	if (list->size == 0)
	{
		list->head = node;
		list->tail = node;
	}
	else
	{
		list->tail->next = node;
		list->tail = node;
	}
	list->size++;
}

void list_fprintf(struct List *list, FILE *output, void (*callback)(FILE *, void *))
{
	struct ListNode *node = list->head;
	while (node != NULL)
	{
		(*callback)(output, node->data);
		node = node->next;
	}
}

void list_foreach(struct List *list, void (*callback)(void *))
{
	struct ListNode *node = list->head;
	while (node != NULL)
	{
		(*callback)(node->data);
		node = node->next;
	}
}

void list_clear(struct List *list)
{
	struct ListNode *node = NULL;
	while (list->head != NULL)
	{
		node = list->head;
		list->head = node->next;
		free(node);
	}
	list->tail = NULL;
	list->size = 0;
}

void list_free(struct List *list)
{
	list_clear(list);
	free(list);
}



void list_clear_foreach(struct List *list, void (*callback)(void *))
{
	struct ListNode *node = NULL;
	while (list->head != NULL)
	{
		node = list->head;
		list->head = node->next;
		(*callback)(node->data);
		free(node);
	}
	list->tail = NULL;
	list->size = 0;
}

void list_free_foreach(struct List *list, void (*callback)(void *))
{
	list_clear_foreach(list, callback);
	free(list);
}
