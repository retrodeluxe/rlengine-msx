#ifndef _RDL_LIST_H_
#define _RDL_LIST_H_

/* simple linked list implementation based on the one from linux */

#include <stddef.h>

#define container_of(_ptr, _type, _member) \
       ((_type *)( (char *)_ptr - offsetof(_type,_member)))

struct list_head {
	struct list_head *next;
	struct list_head *prev;
};

/**
 * list_entry - get the struct for this entry
 * @ptr:	the &struct list_head pointer.
 * @type:	the type of the struct this is embedded in.
 * @member:	the name of the list_head within the struct.
 */
#define list_entry(ptr, type, member) container_of(ptr, type, member)

/**
 * list_for_each	-	iterate over a list
 * @pos:	the &struct list_head to use as a loop cursor.
 * @head:	the head for your list.
 */
#define list_for_each(pos, head) \
	for (pos = (head)->next; pos; pos = pos->next)

extern void INIT_LIST_HEAD(struct list_head *list);
extern void list_add(struct list_head *new, struct list_head *head);
extern void list_del(struct list_head *entry);

#endif
