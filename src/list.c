#include "list.h"

#include <unistd.h>

#include "util.h"

void list_clear(list_head_t *list)
{
	list_item_t *item;
	for (item = list->end; item != NULL; )
	{
		item = item->prev;
		list_del_item(list, item->next);
	}
}



void list_free_item(list_item_t *item)
{
	if (item) {
		if (item->data) free(item->data);
		free(item);
	}
}

void* list_del_item(list_head_t *list, list_item_t *item)
{
	check(list, "List can't be null");
	check(item, "Item can't be null");

	if (item == list->end)
		list->end = item->prev;

	if (item == list->start)
		list->start = item->next;

	if (item->next)
	{
		item->next->prev = item->prev;
	}

	if (item->prev)
		item->prev->next = item->next;
	
error:
	return item->data;
}

void list_del_free_item(list_head_t *list, list_item_t *item)
{
	list_del_item(list, item);	
	list_free_item(item);
}

list_item_t* list_new_item(list_head_t *list)
{
	list_item_t *item = NULL;

	check(list, "list can't be null");
	
	item = malloc(sizeof(list_item_t));
	check(item, "Failed to malloc for item");

	item->next = list->start;
	item->prev = NULL;

	if (list->start) {
		list->start->prev = item;
	} else {
		list->end = item;
	}

	list->start = item;

	return item;
error:
	return NULL;
}

list_head_t* list_new()
{
	list_head_t *list;

	list = malloc(sizeof(list_head_t));
	check(list, "Failed to malloc list");
	list->start = NULL;
	list->end = NULL;

	return list;
error:
	return NULL;
}
