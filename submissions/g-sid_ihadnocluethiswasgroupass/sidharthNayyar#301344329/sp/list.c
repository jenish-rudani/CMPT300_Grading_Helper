#include "list.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void initlist(List *ilist) {

	ilist->front = NULL;
	ilist->rear=NULL;
}


void insertback(List *ilist, char *val) {
	//Listitem *ptr;
	Listitem *newitem = (Listitem *)malloc(sizeof(Listitem));
	//newitem->data = val;
	memcpy(newitem->data,val,sizeof(newitem->data));
	newitem->next = 0;
	
	if(ilist->front == NULL && ilist->rear==NULL){
		ilist->front=ilist->rear=newitem;
	}else{
		ilist->rear->next=newitem;
		ilist->rear=newitem;
	}
}

void removeFromList(List *ilist){
    Listitem *temp;

    if(ilist->front == NULL)
         printf("Queue is Empty. Unable to perform dequeue\n");
    else
    {
        //take backup
        temp = ilist->front;

        //make the front node points to the next node
        //logically removing the front element
        ilist->front = ilist->front->next;

        //if front == NULL, set rear = NULL
        if(ilist->front == NULL)
            ilist->rear = NULL;

       //free the first node
       free(temp);
    }

}

void printList(List *ilist){

    Listitem *temp = ilist->front;

    while(temp)
    {
        printf("%s->",temp->data);
        temp = temp->next;
    }
    printf("NULL\n");

}
void destroy(List *ilist) { 
	Listitem *ptr1,
	*ptr2;
	if (!ilist->front) return; 
	ptr1 = ilist->front; 
	while (ptr1) {
	ptr2 = ptr1;
	ptr1 = ptr1->next;
	free(ptr2);
	}
	ilist->front = 0;
}

int length(List ilist){ 
	Listitem *ptr;
	int count = 1;
	if (!ilist.front) return 0;
	ptr = ilist.front;
	while (ptr->next) {
	ptr = ptr->next;
	count++;
	}
	return count;
}
/*
int getitem(List ilist, int n) {

Listitem *ptr;
int count = 0;
if (!ilist.front) return 0;
ptr = ilist.front;
if (n==0) return ptr->data;
while (ptr->next) {
ptr = ptr->next;
count++;
if (n == count)
return (ptr->data);
}
return 0;
}*/



/*	
void insertfront(List *ilist, int val) {

	Listitem *newitem;
	newitem = (Listitem *)malloc(sizeof(Listitem));
	newitem->next = ilist->front;
	newitem->data = val;
	ilist->front = newitem;
}*/
