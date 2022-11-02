#ifndef _LIST_H
#define _LIST_H

struct listitem {
	//int data;
	char data[500];
	struct listitem *next;
};

struct listitem *front, *rear;

typedef struct listitem Listitem;
struct list {
Listitem *front;
Listitem *rear;
};
typedef struct list List;
void initlist (List *); /* initialize an empty list */

void insertback(List *, char *val); /* insert val at back */

int length(List); /* returns list length */

void destroy(List *); /* deletes list */
//void insertfront(List * , int val); /* insert val at front */
void printList(List *);
void removeFromList(List *ilist);



//void setitem(List *, int n, int val);/* modifies item at n to val*/
//int getitem(List, int n); /* returns value at n*/
#endif /* _LIST_H */
