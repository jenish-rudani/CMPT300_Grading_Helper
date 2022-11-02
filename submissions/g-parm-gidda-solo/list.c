/*
Parm Gidda - 301423241 - parm_gidda@sfu.ca
*/

#include <stdio.h>
#include "list.h"
#include <assert.h>


static bool firstList = true;
static int listsInUse = 0;
static int nodesInUse = 0;
static struct List_s allLists[LIST_MAX_NUM_HEADS];
static struct Node_s allNodes[LIST_MAX_NUM_NODES];


List* List_create(){

	

	//on first list_create call initialize all list heads and nodes 
	if(firstList == true){

		for(int i = 0; i < LIST_MAX_NUM_HEADS; i++){
			allLists[i].amountOfItems = 0;
			allLists[i].currentItem = NULL;
			allLists[i].head = NULL;
			allLists[i].tail = NULL;
			allLists[i].currentItemIndex = -1;
			allLists[i].available = true;
		}

		for(int i = 0; i < LIST_MAX_NUM_NODES; i++){
			allNodes[i].item = NULL;
			allNodes[i].nextItem = NULL;
			allNodes[i].previousItem = NULL;

		}

		listsInUse++;
		allLists[0].available = false;
		firstList = false;
		return &(allLists[0]);
	}
	//no more head available
	else if(listsInUse == LIST_MAX_NUM_HEADS){
		return NULL;
	}
	//find an empty head
	else{
		

		int emptyList = find_empty_list();
		listsInUse++;
		allLists[emptyList].amountOfItems = 0;
		allLists[emptyList].currentItem = NULL;
		allLists[emptyList].head = NULL;
		allLists[emptyList].tail = NULL;
		allLists[emptyList].currentItemIndex = -1;
		allLists[emptyList].available = false;

		return &(allLists[emptyList]);
	}

}

//get amount of items in list
int List_count(List* pList){

	return pList->amountOfItems;

}


void* List_first(List* pList){

	//If list is empty return null or return the head of the list
	if(List_count(pList) == 0){

		pList->currentItem = NULL;
		return NULL;
	}
	else{
		pList->currentItem = pList->head;
		pList->currentItemIndex = 0;
		return pList->currentItem->item;

	}
}

//get the current amount of active nodes
int getNodesInUse(){
	return nodesInUse;
}

//get the current amount of active heads
int getListsInUse(){
	return listsInUse;
}

void* List_last(List* pList){

	//if list is empty return null or return the tail of the list
	if(List_count(pList) == 0){

		pList->currentItem = NULL;
		return NULL;
	}
	else{
		pList->currentItem = pList->tail;
		pList->currentItemIndex = pList->amountOfItems-1;
		return pList->currentItem->item;

	}
}

void* List_next(List* pList){


	//empty list return null
	if(List_count(pList) == 0){

	
		pList->currentItemIndex -1;
		return NULL;
	}
	//At last item in list or outside of end
	else if(pList->currentItemIndex >= pList->amountOfItems-1){
		
		pList->currentItemIndex = pList->amountOfItems;
		return NULL;
	}
	//if outside of start return head
	else if(pList->currentItemIndex == -1){

		
		pList->currentItem = pList->head;
		pList->currentItemIndex = 0;

		return pList->currentItem->item;
	}
	//return the next item from the current item
	else{
	

		pList->currentItem = pList->currentItem->nextItem;

		pList->currentItemIndex = pList->currentItemIndex + 1;

		return pList->currentItem->item;
	}
}

void* List_prev(List* pList){

	//empty list return null
	if(List_count(pList) == 0){
		pList->currentItemIndex -1;

		return NULL;
	}
	//outside of start or first item
	else if(pList->currentItemIndex <= 0){
		pList->currentItemIndex = -1;
	
		return NULL;
	}
	//At last item in list
	else if(pList->currentItemIndex == pList->amountOfItems){
		pList->currentItem = pList->tail;
		pList->currentItemIndex = pList->amountOfItems-1;
		return pList->currentItem->item;
	}
	//return the previous item from the current item
	else{
		pList->currentItem = pList->currentItem->previousItem;
		pList->currentItemIndex = pList->currentItemIndex -1;

		return pList->currentItem->item;
	}
}

void* List_curr(List* pList){

	//if outside of list return null otherewise return the current item
	if(pList->currentItemIndex == -1 || pList->currentItemIndex == pList->amountOfItems){
		
		return NULL;
	}
	else{
		return pList->currentItem->item;
	}

}

int List_insert_after(List* pList, void* pItem){

	int nodeIndex = -2;

	//fail insert if no more nodes available or list has not been properly initialized
	if(nodesInUse == LIST_MAX_NUM_NODES || pList == NULL){
		return -1;
	}
	else{
	
		//get the index of an available node
		nodeIndex = find_empty_node();

		//if list is empty head and tail or set to be equal
		if(pList->amountOfItems == 0){

			allNodes[nodeIndex].item = pItem;
			allNodes[nodeIndex].nextItem = pList->head;
			allNodes[nodeIndex].previousItem = NULL;

			pList->currentItem = &allNodes[nodeIndex];
			pList->head = &allNodes[nodeIndex];
			pList->tail = &allNodes[nodeIndex];
			pList->amountOfItems = pList->amountOfItems + 1;
			pList->currentItemIndex = 0;
		}
		else{

			//if current is outside of start new node will become head
			if(pList->currentItemIndex == -1){

				
				allNodes[nodeIndex].item = pItem;
				allNodes[nodeIndex].nextItem = pList->head;
				allNodes[nodeIndex].previousItem = NULL;

				pList->head->previousItem = &allNodes[nodeIndex];

				//if only 1 item in list then current head becomes tail
				if(pList->amountOfItems == 1){
					pList->tail = pList->head;
				}

			
				pList->head = &allNodes[nodeIndex];
				pList->currentItem = &allNodes[nodeIndex];
				pList->amountOfItems = pList->amountOfItems + 1;
				pList->currentItemIndex = 0;		
			
			}
			//if at the outside of end or at the last element in array the new node will become the tail
			else if(pList->currentItemIndex == pList->amountOfItems || pList->currentItemIndex == pList->amountOfItems -1){
			
				allNodes[nodeIndex].item = pItem;
				allNodes[nodeIndex].nextItem = NULL;

				//if the only 1 item in list then head must be linked to the new node directly 
				if(pList->amountOfItems == 1){
		
					allNodes[nodeIndex].previousItem = pList->head;
					pList->head->nextItem = &allNodes[nodeIndex];

				}
				else{
					allNodes[nodeIndex].previousItem = pList->tail;
					pList->tail->nextItem = &allNodes[nodeIndex];
				}
			
				pList->tail = &allNodes[nodeIndex];
				pList->currentItem = &allNodes[nodeIndex];
				pList->amountOfItems = pList->amountOfItems + 1;
				pList->currentItemIndex = pList->amountOfItems-1;

			}
			//link new node anywhere else inside of list
			else{

				allNodes[nodeIndex].item = pItem;
				allNodes[nodeIndex].nextItem = pList->currentItem->nextItem;
				allNodes[nodeIndex].previousItem = pList->currentItem;

				pList->currentItem->nextItem->previousItem = &allNodes[nodeIndex];
				pList->currentItem->nextItem = &allNodes[nodeIndex];
				pList->currentItem = &allNodes[nodeIndex];
				pList->amountOfItems = pList->amountOfItems + 1;
				pList->currentItemIndex = pList->currentItemIndex + 1;

			}
		}
		nodesInUse++;
		return 0;
	}	
}

//search node array for a node which is available and return its index
int find_empty_node(){

	for(int i = 0; i < LIST_MAX_NUM_NODES; i++){

		if(allNodes[i].item == NULL){
			return i;
		} 
	}
	return -1;
}

//search list array for a list which is available and return its index
int find_empty_list(){

	for(int i = 0; i < LIST_MAX_NUM_HEADS; i++){

		if(allLists[i].available == true){
			return i;
		} 
	}
	return -1;
}

//print list for testing purposes
void printTest(List* pList){

	List_first(pList);
	for(int i = 0; i < pList->amountOfItems; i++){

		printf("------------\n");
		printf("Test: %s\n", (char*)(pList->currentItem->item));
		printf("----===----\n");	
		List_next(pList);

	}
	printf("\n\n\n\n");
}

//exact oppoiste operations as List_insert_after function
int List_insert_before(List* pList, void* pItem){
		int nodeIndex = -2;

	if(nodesInUse == LIST_MAX_NUM_NODES || pList == NULL){
		return -1;
	}
	else{

		nodeIndex = find_empty_node();

		if(pList->amountOfItems == 0){

			allNodes[nodeIndex].item = pItem;
			allNodes[nodeIndex].nextItem = pList->head;
			allNodes[nodeIndex].previousItem = NULL;

			pList->currentItem = &allNodes[nodeIndex];
			pList->head = &allNodes[nodeIndex];
			pList->tail = &allNodes[nodeIndex];
			pList->amountOfItems = pList->amountOfItems + 1;
			pList->currentItemIndex = 0;
		}
		else{

			if(pList->currentItemIndex == -1 || pList->currentItemIndex == 0){

				
				allNodes[nodeIndex].item = pItem;
				allNodes[nodeIndex].previousItem = NULL;

					if(pList->amountOfItems == 1){
		
					allNodes[nodeIndex].nextItem = pList->tail;
					pList->tail->previousItem = &allNodes[nodeIndex];

				}
				else{
					allNodes[nodeIndex].nextItem = pList->head;
					pList->head->previousItem = &allNodes[nodeIndex];
				}

						
				pList->head = &allNodes[nodeIndex];
				pList->currentItem = &allNodes[nodeIndex];
				pList->amountOfItems = pList->amountOfItems + 1;
				pList->currentItemIndex = 0;

			}
			else if(pList->currentItemIndex == pList->amountOfItems){
			
				allNodes[nodeIndex].item = pItem;
				allNodes[nodeIndex].nextItem = NULL;
				allNodes[nodeIndex].previousItem = pList->tail;

				pList->tail->nextItem = &allNodes[nodeIndex];

				
				if(pList->amountOfItems == 1){
					pList->head = pList->tail;
				}
			
				pList->tail = &allNodes[nodeIndex];
				pList->currentItem = &allNodes[nodeIndex];
				pList->amountOfItems = pList->amountOfItems + 1;
				pList->currentItemIndex = pList->amountOfItems-1;

			}
			else{


				allNodes[nodeIndex].item = pItem;
				allNodes[nodeIndex].nextItem = pList->currentItem;
				allNodes[nodeIndex].previousItem = pList->currentItem->previousItem;

				pList->currentItem->previousItem->nextItem = &allNodes[nodeIndex];
				pList->currentItem->previousItem = &allNodes[nodeIndex];
				pList->currentItem = &allNodes[nodeIndex];
				pList->amountOfItems = pList->amountOfItems + 1;
				pList->currentItemIndex = pList->currentItemIndex -1;

			}
		}
		nodesInUse++;
		return 0;
	}	
}


int List_append(List* pList, void* pItem){
			int nodeIndex = -2;



	//fail if no more available nodes or list is not initalized properly
	if(nodesInUse == LIST_MAX_NUM_NODES || pList == NULL){
		return -1;
	}
	//same code as insert at end if statement in List_insert_after function.
	else{

		nodeIndex = find_empty_node();

		if(pList->amountOfItems == 0){


			(allNodes[nodeIndex].item) = pItem;

			allNodes[nodeIndex].nextItem = pList->head;
			allNodes[nodeIndex].previousItem = NULL;

			pList->currentItem = &allNodes[nodeIndex];
			pList->head = &allNodes[nodeIndex];
			pList->tail = &allNodes[nodeIndex];
			pList->amountOfItems = pList->amountOfItems + 1;
			pList->currentItemIndex = 0;
		}
		else{

				(allNodes[nodeIndex].item) = pItem;
				allNodes[nodeIndex].nextItem = NULL;

				if(pList->amountOfItems == 1){
		
					allNodes[nodeIndex].previousItem = pList->head;
					pList->head->nextItem = &allNodes[nodeIndex];

				}
				else{
					allNodes[nodeIndex].previousItem = pList->tail;
					pList->tail->nextItem = &allNodes[nodeIndex];
				}
			
				pList->tail = &allNodes[nodeIndex];
				pList->currentItem = &allNodes[nodeIndex];
				pList->amountOfItems = pList->amountOfItems + 1;
				pList->currentItemIndex = pList->amountOfItems-1;

		}
		nodesInUse++;
		return 0;
	}	

}

int List_prepend(List* pList, void* pItem){
		int nodeIndex = -2;

	//fail if no more available nodes or list is not initalized properly
	if(nodesInUse == LIST_MAX_NUM_NODES || pList == NULL){
		return -1;
	}
	//same code as insert at start if statement in List_insert_before function.
	else{

		nodeIndex = find_empty_node();

		if(pList->amountOfItems == 0){

			allNodes[nodeIndex].item = pItem;
			allNodes[nodeIndex].nextItem = pList->head;
			allNodes[nodeIndex].previousItem = NULL;

			pList->currentItem = &allNodes[nodeIndex];
			pList->head = &allNodes[nodeIndex];
			pList->tail = &allNodes[nodeIndex];
			pList->amountOfItems = pList->amountOfItems + 1;
			pList->currentItemIndex = 0;
		}
		else{

				allNodes[nodeIndex].item = pItem;
				allNodes[nodeIndex].previousItem = NULL;

					if(pList->amountOfItems == 1){
		
					allNodes[nodeIndex].nextItem = pList->tail;
					pList->tail->previousItem = &allNodes[nodeIndex];

				}
				else{
					allNodes[nodeIndex].nextItem = pList->head;
					pList->head->previousItem = &allNodes[nodeIndex];
				}

						
				pList->head = &allNodes[nodeIndex];
				pList->currentItem = &allNodes[nodeIndex];
				pList->amountOfItems = pList->amountOfItems + 1;
				pList->currentItemIndex = 0;			
		
		}
		nodesInUse++;
		return 0;
	}	
}

void* List_remove(List* pList){

	//return null if current item is outside of list
	if(pList->currentItemIndex == -1 || pList->currentItemIndex == pList->amountOfItems){
		return NULL;
	}
	else{

		//if only 1 item in list tail and head must be removed
		if(pList->amountOfItems == 1){

			void* copyOfItemData = pList->currentItem->item;
			pList->head->item = NULL;
			pList->tail->item = NULL;
			pList->currentItem->item = NULL;
			pList->amountOfItems = 0;
			pList->currentItemIndex = -1;

			nodesInUse--;
			return copyOfItemData;
		}

		//remove last item in list
		if(pList->currentItemIndex == pList->amountOfItems -1){

			

			void* copyOfItemData = pList->tail->item;
			//remove last item
			pList->tail->item = NULL;
			pList->tail = pList->tail->previousItem;
			pList->tail->nextItem = NULL;


			pList->currentItemIndex = pList->amountOfItems - 2;
			pList->currentItem = pList->tail;
			pList->amountOfItems = pList->amountOfItems - 1;

			nodesInUse--;
			return copyOfItemData;

		}
		//remove first item in list
		else if(pList->currentItemIndex == 0){

			void* copyOfItemData = pList->head->item;
			pList->head->item = NULL;
			pList->head = pList->head->nextItem;
			pList->head->previousItem = NULL;

			pList->currentItemIndex = 0;
			pList->currentItem = pList->head;
			pList->amountOfItems = pList->amountOfItems -1;

			nodesInUse--;
			return copyOfItemData;

		}
		//remove any other item
		else{

			void* copyOfItemData = pList->currentItem->item;
			pList->currentItem->previousItem->nextItem = pList->currentItem->nextItem;
			pList->currentItem->nextItem->previousItem = pList->currentItem->previousItem;

			pList->currentItemIndex = pList->currentItemIndex;
			pList->currentItem->item = NULL;
			pList->currentItem = pList->currentItem->nextItem;
			pList->amountOfItems = pList->amountOfItems - 1;

		
			nodesInUse--;
			return copyOfItemData;

		}
	}
}

void* List_trim(List* pList){

	if(pList->amountOfItems == 0){
		return NULL;
	}
	//same code from remove last item in list section of List_remove function
	else if(pList->amountOfItems == 1){

			void* copyOfItemData = pList->currentItem->item;
			pList->head->item = NULL;
			pList->tail->item = NULL;
			pList->currentItem->item = NULL;
			pList->amountOfItems = 0;
			pList->currentItemIndex = -1;

			nodesInUse--;
			return copyOfItemData;
	}
	else{

			void* copyOfItemData = pList->tail->item;
			//remove last item
			pList->tail->item = NULL;
			pList->tail = pList->tail->previousItem;
			pList->tail->nextItem = NULL;

			pList->amountOfItems = pList->amountOfItems - 1;
			pList->currentItemIndex = pList->amountOfItems - 1;
			pList->currentItem = pList->tail;
			
			nodesInUse--;
			return copyOfItemData;

	}
}

void List_concat(List* pList1, List* pList2){

	//if both lists empty do nothing 
	if(pList1->amountOfItems == 0 && pList2->amountOfItems == 0){}

	//if pList1 empty copy over everything from pList2
	else if(pList1->amountOfItems == 0){

		pList1->head = pList2->head;
		pList1->tail = pList2->tail;
		pList1->currentItemIndex = -1;
		pList1->amountOfItems = pList2->amountOfItems;
	}
	//if pList2 empty do nothing
	else if(pList2->amountOfItems == 0){}

	//combine both lists
	else{

		pList1->tail->nextItem = pList2->head;
		pList2->head->previousItem = pList1->tail;
		pList1->tail = pList2->tail;

		pList1->amountOfItems = pList1->amountOfItems + pList2->amountOfItems;

	}

	//make pList2's head available again
	pList2->available = true;
	pList2->currentItem = NULL;
	pList2->head = NULL;
	pList2->tail = NULL;
	pList2->amountOfItems = 0;
	pList2->currentItemIndex = -1;
	pList2 = NULL;
	listsInUse--;
}


void List_free(List* pList, FREE_FN pItemFreeFn){

	List_first(pList);

	for(int i = 0; i < pList->amountOfItems; i++){

		
		(*pItemFreeFn)(pList->currentItem->item);

		pList->currentItem->item = NULL;

		List_next(pList);

	}

	//make pList's head and nodes available again
	nodesInUse = nodesInUse - pList->amountOfItems;
	pList->available = true;
	pList->currentItem = NULL;
	pList->head = NULL;
	pList->tail = NULL;
	pList->amountOfItems = 0;
	pList->currentItemIndex = -1;
	pList = NULL;
	listsInUse--;
}

void* List_search(List* pList, COMPARATOR_FN pComparator, void* pComparisonArg){

	if(pList->currentItemIndex == -1){
		List_first(pList);
	}

	//search list for a match and return the matched item or Null is returned
	for(int i = pList->currentItemIndex; i < pList->amountOfItems; i++){

		if((*pComparator)(pList->currentItem->item, pComparisonArg)){

			return pList->currentItem->item;
		}

		List_next(pList);
		
	}
	pList->currentItemIndex = pList->amountOfItems;
	return NULL;
}



