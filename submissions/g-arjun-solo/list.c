#include<stdio.h>
#include<assert.h>
#include<stdlib.h>
#include<stddef.h>
#include "list.h"
#include<string.h>
#include<stdbool.h>


/*
ASSIGNMENT1 BY ARJUN SINGH

I had some personal issues to deal with and had trouble getting this assignment done on time. I didn't get to work on it
as much as possible my apologies. Won't happen again in future assignments!


*/

// Maximum number of unique lists the system can support

// (You may modify this, but reset the value to 10 when handing in your assignment)

#define LIST_MAX_NUM_HEADS 10


// Maximum total number of nodes (statically allocated) to be shared across all lists

// (You may modify this, but reset the value to 100 when handing in your assignment)

#define LIST_MAX_NUM_NODES 100

static bool startupInitialization;

//arrays containing nodes and list heads
List arrayList[LIST_MAX_NUM_HEADS];
Node arrayNode[LIST_MAX_NUM_NODES];

//moved to list.c
static List* firstFreeList;
static Node* firstFreeNode;


//the size of the amount of nodes and heads(lists) each array can hold
static int headsAvailable;
static int nodesAvailable;

//blank nodes and list to initialize lists with

//blank nodes and list to initialize lists with
static Node blankNode = {NULL, NULL, NULL};
static List blankList = {NULL, NULL, NULL, -1, 0};

/* free list and node utility functions */



void clearList(List *pList) {
	//resets lists to blankList ^^^^
	//clears list item then makes it cleared off array
	pList->head = NULL;
    pList->currentItem  = NULL;
    pList->tail  = NULL;
    pList->size = 0;
	pList->boundary = 0;
	headsAvailable++;

}


void clearNode(Node *pNode) {
	//resets node to blankNode ^^^^
	pNode->item = NULL;
	pNode->prev = NULL;
	nodesAvailable++;
}



List* startup() {
		//NODE ARRAY LIST INITIALIZATION
	firstFreeNode = arrayNode;
	//whenever this is called startup of a list is set to true so a new list is initialized. Otherwise we return null.
	if(startupInitialization == true) {
		for(int i = 0; i < LIST_MAX_NUM_NODES; i++) {
			//initialize first free nodes with null nodes
			firstFreeNode[i] = blankNode;
			//initialize arraynodes with free blank nodes
			arrayNode[i] = firstFreeNode[i];

		}
		firstFreeNode[LIST_MAX_NUM_NODES-1].next = NULL;
		nodesAvailable = (LIST_MAX_NUM_NODES);

		firstFreeList = arrayList;
		for(int i = 0; i < LIST_MAX_NUM_HEADS; i++) {
			//initialize first free heads with null lists
			firstFreeList[i] = blankList;
			//initialize list with first free lists
			arrayList[i] = firstFreeList[i];

		}
		firstFreeNode[LIST_MAX_NUM_NODES-1].next = NULL;
		headsAvailable = (LIST_MAX_NUM_HEADS);

		List* nList = &arrayList[headsAvailable];
		startupInitialization = false;
		return nList;
	}
	return NULL;
}

// General Error Handling:

// Client code is assumed never to call these functions with a NULL List pointer, or 

// bad List pointer. If it does, any behaviour is permitted (such as crashing).

// HINT: Use assert(pList != NULL); just to add a nice check, but not required.



// Makes a new, empty list, and returns its reference on success. 

// Returns a NULL pointer on failure.

List* List_create(){
	// if(checkListsExhausted()){
	// 	return NULL;
	// }
	startupInitialization = true;
	List *temp = startup();
	return temp;
}



// Returns the number of items in pList.

int List_count(List* pList){
	//return list size
	return pList->size;

}



// Returns a pointer to the first item in pList and makes the first item the current item.

// Returns NULL and sets current item to NULL if list is empty.

//DONE

void* List_first(List* pList){
	if(pList == NULL){
		return NULL;
	}
	if(pList->size == 0) {
		pList->currentItem = NULL;
		return NULL;
	}

	pList->currentItem = pList->head;
	return pList->currentItem->item;
}



// Returns a pointer to the last item in pList and makes the last item the current item.

// Returns NULL and sets current item to NULL if list is empty.

void* List_last(List* pList){
	if(pList == NULL){
		return NULL;
	}

	if(pList->size == 0) {
		pList->currentItem = NULL;
		return NULL;
	}

	pList->currentItem = pList->tail;
	return pList->currentItem->item;

}



// Advances pList's current item by one, and returns a pointer to the new current item.

// If this operation advances the current item beyond the end of the pList, a NULL pointer 

// is returned and the current item is set to be beyond end of pList.

void* List_next(List* pList){
	if(pList == NULL || (pList->size == 0)){
		return NULL;
		//means that 
	}
	 if(pList->currentItem->next == NULL || (pList-> currentItem == pList->tail)){
	 //make beyond functionality when the next item is out of bounds
	 	pList->boundary = LIST_OOB_END;
		pList->currentItem = NULL;
		return NULL;
	}
	//if the boundary is before the first item i nthe list
	if(pList->boundary == LIST_OOB_START) {
		pList->currentItem = pList->head;
		return pList->currentItem->item;
	}

	pList->currentItem = pList->currentItem->next;
	return pList->currentItem->item;

}



// Backs up pList's current item by one, and returns a pointer to the new current item. 

// If this operation backs up the current item beyond the start of the pList, a NULL pointer 

// is returned and the current item is set to be before the start of pList.

void* List_prev(List* pList){
	if(pList == NULL || (pList->size == 0)){
			return NULL;
	}
	//case if the current item is the head aka also has a null ptr on its prev pointer
	if(pList->currentItem->prev == NULL || (pList-> currentItem == pList->head)){
	 //make beyond functionality when the next item is out of bounds
	 	pList->boundary = LIST_OOB_START;
		pList->currentItem = NULL;
		return NULL;
	}
	//if the boundary is beyond the list
	if(pList->boundary == LIST_OOB_END) {
		pList->currentItem = pList->tail;
		return pList->currentItem->item;
	}
	pList->currentItem = pList->currentItem->prev;
	return pList->currentItem->item;
}



// Returns a pointer to the current item in pList.

void* List_curr(List* pList) {

	if(pList == NULL){
		return NULL;
	}
	//if boundary is beyond list, return null
	if(pList->boundary == LIST_OOB_END) {
		return NULL;
	}
	//if boundary is before first node, return nul
	if(pList->boundary == LIST_OOB_START) {
		return NULL;
	}
	return pList->currentItem->item;
}



// Adds the new item to pList directly after the current item, and makes item the current item. 

// If the current pointer is before the start of the pList, the item is added at the start. If 

// the current pointer is beyond the end of the pList, the item is added at the end. 

// Returns 0 on success, -1 on failure.

int List_insert_after(List* pList, void* pItem) {

	if(pList == NULL) {
		return -1;
	}

	if(pList->size == 0) {
		Node *addNode = &arrayNode[nodesAvailable];
		pList->currentItem = addNode;
        addNode->item = pItem;
        pList->head = addNode;
        addNode = pList->tail;
        pList->size++;
        return 0;
	}


//if boundary reaches beyond end of list
	if(pList->boundary == LIST_OOB_END){
		//for the beyond the list, the added items next point must be null since its the end
		Node *addNode = &arrayNode[nodesAvailable];
        addNode->item = pItem;
		addNode->next = NULL;
		addNode->prev = pList->tail;
		pList->tail->next = addNode;
		pList->currentItem = addNode;
		pList->tail = addNode;
		pList->size++;
		return 0;
	}	
	//if boundary reaches before the start of the list
	else if(pList->boundary == LIST_OOB_START){
		//added items prev pointer must be null since its the first pointer
		Node *addNode = &arrayNode[nodesAvailable];
        addNode->item = pItem;
		addNode->prev = NULL;
		addNode->next = pList->head;
		pList->head->next = addNode;
		pList->currentItem = addNode;
		pList->head = addNode;
		pList->size++;
        return 0;
	}	
	//otherwise if its a regular list we add it like a normal linked list insertion but with the current node
	else {
		Node *addNode = &arrayNode[nodesAvailable];
        addNode->item = pItem;
        addNode = pList->currentItem->next;
        addNode->prev = pList->currentItem;
        pList->currentItem = addNode;
        pList->size++;
        return 0;

	}

	//nodes available within the array go down
	nodesAvailable--;
	return 0;

}



// Adds item to pList directly before the current item, and makes the new item the current one. 

// If the current pointer is before the start of the pList, the item is added at the start. 

// If the current pointer is beyond the end of the pList, the item is added at the end. 

// Returns 0 on success, -1 on failure.

int List_insert_before(List* pList, void* pItem){

	
	if(pList == NULL) {
		return -1;
	}


	if(pList->size == 0) {
		Node *addNode = &arrayNode[nodesAvailable];
		pList->currentItem = addNode;
        addNode->item = pItem;
        pList->head = addNode;
        addNode = pList->tail;
        pList->size++;
        return 0;
	}


//if boundary reaches beyond end of list
	if(pList->boundary == LIST_OOB_END){
		//added items prev pointer must be null since its the first pointer
		Node *addNode = &arrayNode[nodesAvailable];
        addNode->item = pItem;
		addNode->next = NULL;
		addNode->prev = pList->tail;
		pList->tail->next = addNode;
		pList->currentItem = addNode;
		pList->tail = addNode;
		pList->size++;
		return 0;
	}	
	//if boundary reaches before the start of the list
	else if(pList->boundary == LIST_OOB_START){
		//added items prev pointer must be null since its the first pointer
		Node *addNode = &arrayNode[nodesAvailable];
        addNode->item = pItem;
		addNode->prev = NULL;
		addNode->next = pList->head;
		pList->head->next = addNode;
		pList->currentItem = addNode;
		pList->head = addNode;
		pList->size++;
        return 0;
	}	
	//otherwise if its a regular list, we add the new node before the current item and set the current item to the new node
	else {
		Node *addNode = &arrayNode[nodesAvailable];
        addNode->item = pItem;
        addNode = pList->currentItem->prev;
        addNode->next = pList->currentItem;
        pList->currentItem = addNode;
        pList->size++;
        return 0;

	}

	nodesAvailable--;
	return 0;

}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Adds item to the end of pList, and makes the new item the current one. 

// Returns 0 on success, -1 on failure.

int List_append(List* pList, void* pItem){
	 if ((pList == NULL) ){
        return -1;
    } else if(pList->size == 0){
		//if the size is 0, we simply add it like it normally should be added with insert after
		List_insert_after(pList, pItem);
		return 0;
	}

	//set plist current item to the tail

	List_last(pList);
	//sends list with current item set as the tail alongside new item to insert after function
	//List_insert_after(pList, pItem)
	List_insert_after(pList,pItem);
	return 0;
	//make new list node variable becuase you cant allocate it
}



// Adds item to the front of pList, and makes the new item the current one. 

// Returns 0 on success, -1 on failure.

int List_prepend(List* pList, void* pItem){
	if ((pList == NULL) ){
        return -1;
    } else if(pList->size == 0){
		//if the list is empty we add the item as it nromally should be added, no need for the list_first function call
		List_insert_before(pList, pItem);
		return 0;
	}
	//sets current item in passed list to the first item

	List_first(pList);
	//sends current(head) item of list and item to add to list to insert before function
	List_insert_before(pList, pItem);
	return 0;
}



// Return current item and take it out of pList. Make the next item the current one.

// If the current pointer is before the start of the pList, or beyond the end of the pList,

// then do not change the pList and return NULL.

void* List_remove(List* pList){

	if ((pList == NULL) || (pList->size == 0) )
    {
        return NULL;
    }

	if(pList->boundary == LIST_OOB_END || pList->boundary == LIST_OOB_START) {
		return NULL;
	}

	//make variable to hold the current of the list that will be returned 
	Node *removedNode;
	void *removedNodeItem;
	removedNode = pList->currentItem;
	removedNodeItem = removedNode->item;
	pList->size--;

	//head == tail
	if(pList->size == 1) {
		removedNode = pList->head;
		removedNodeItem = removedNode->item;
	}

	//as long as the currentitem is not hte tail!
	if(pList->currentItem != pList->tail) {
		pList->currentItem = pList->currentItem->next;
	}

	//if the current item is the head
	if(pList->currentItem == pList->head) {
		pList->head = pList->currentItem;
	}

	//if the current items next item is not null, we set the next node to its current node because we remove the current node
	if (pList->currentItem->next != NULL) {
		removedNode->next->prev = removedNode->prev;
	}

	//we change currentitem->prev if current is not the first item
	if(pList->currentItem->prev != NULL) {
		removedNode->prev->next = removedNode->next;
	}

	//free up memory
	return removedNodeItem;

}



// Return last item and take it out of pList. Make the new last item the current one.

// Return NULL if pList is initially empty.

void* List_trim(List* pList){

	if ((pList == NULL) || (pList->size == 0) )
    {
        return NULL;
    }

    if(pList->currentItem == NULL) {
    	return NULL;
    }

    if(pList->boundary == LIST_OOB_END || pList->boundary == LIST_OOB_START) {
    	return NULL;
    }

	//make variable to hold the last item of the list that will be returned
	nodesAvailable += 1;
	Node *trimmedNode = &arrayNode[nodesAvailable];
	void* trimmedNodeItem;
	trimmedNode = pList->currentItem;
	trimmedNodeItem = trimmedNode->item;

	//delete last item
	pList->tail = pList->tail->prev;
	pList->tail->next = NULL;
	pList->size--;

	//sets new last item as current one
	pList->currentItem = pList->tail;
	return trimmedNodeItem;



}



// Adds pList2 to the end of pList1. The current pointer is set to the current pointer of pList1. 

// pList2 no longer exists after the operation; its head is available

// for future operations.

void List_concat(List* pList1, List* pList2){

//base case
	if(pList1 != NULL && pList2 != NULL) {
		pList1->size = (pList1->size + pList2->size);
		pList2->head->prev = pList1->tail;
		pList1->tail->next = pList2->head;
		pList2->currentItem = pList1->currentItem;
		//free list from the array
//edge case: list 1 is empty, and list 2 is not
	} else if(pList1->size == 0 && pList2 != NULL){
		pList1->size = pList2->size;
		pList1->head = pList2->head;
		pList1->tail = pList2->tail;
		pList1->size = pList2->size;
		pList1->currentItem = pList2->currentItem;

	}
	//edge case: list 1 is empty, and list 2 is not
	else if(pList2->size == 0 && pList1 != NULL){
		pList2->size = pList1->size;
		pList2->head = pList1->head;
		pList2->tail = pList1->tail;
		pList2->size = pList1->size;
		pList2->currentItem = pList1->currentItem;

	}

}



// Delete pList. pItemFreeFn is a pointer to a routine that frees an item. 

// It should be invoked (within List_free) as: (*pItemFreeFn)(itemToBeFreedFromNode);

// pList and all its nodes no longer exists after the operation; its head and nodes are 

// available for future operations.

typedef void (*FREE_FN)(void* pItem);

void List_free(List* pList, FREE_FN pItemFreeFn){
	
}



// Search pList, starting at the current item, until the end is reached or a match is found. 

// In this context, a match is determined by the comparator parameter. This parameter is a

// pointer to a routine that takes as its first argument an item pointer, and as its second 

// argument pComparisonArg. Comparator returns 0 if the item and comparisonArg don't match, 

// or 1 if they do. Exactly what constitutes a match is up to the implementor of comparator. 

// 

// If a match is found, the current pointer is left at the matched item and the pointer to 

// that item is returned. If no match is found, the current pointer is left beyond the end of 

// the list and a NULL pointer is returned.

// 

// If the current pointer is before the start of the pList, then start searching from

// the first node in the list (if any).

typedef bool (*COMPARATOR_FN)(void* pItem, void* pComparisonArg);

void* List_search(List* pList, COMPARATOR_FN pComparator, void* pComparisonArg);
// 	 if ((pList == NULL) ){
//         return -1;
//     } else if(pComparator == NULL) {
//     	return -1;
//     }

// // If no match is found, the current pointer is left beyond the end of the list and a NULL pointer is returned.
//     if(pList->boundary == LIST_OOB_END && pList->currentItem == NULL) {
//     	return NULL
//     }
//     // If the current pointer is before the start of the pList, then start searching from the first node in the list (if any).
//     if(pList->boundary == LIST_OOB_START && pList->currentItem == NULL) {
//     	return List_next(pList);
//     }

//     while(pList->currentItem != NULL) {
//     	if((pComparator)(pList, pComparisonArg)){
//     		return pList->currentItem->item;
//     	}
//     	pList->currentItem = pList->currentItem->next;
//     }
//     return NULL;
// }




