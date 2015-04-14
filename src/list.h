#ifndef LIST_H
#define LIST_H

/* Simple linked list implementation
 * Might not be the best for a wm
 * But I felt like writing one
 * */

typedef struct list_item_t 
{
	void *data; /* This memory is initialize by the user and freed by the list implementation */
	struct list_item_t *next;
	struct list_item_t *prev;
} list_item_t;

typedef struct list_head_t
{
	struct list_item_t *start;
	struct list_item_t *end;
} list_head_t;

/* Remove all items in a list */
void list_clear(list_head_t *list);

/* Frees list and the list contents */
void list_destroy(list_head_t *list);

/* Frees up the memory of a list item, for internal use */
void list_free_item(list_item_t *item);

/* Deletes an item from a list */
void list_del_item(list_head_t *list, list_item_t *item);

/* Make space for a new list item */
list_item_t* list_new_item(list_head_t *list);

/* Creates new list */
list_head_t* list_new();

#endif /* LIST_H */
